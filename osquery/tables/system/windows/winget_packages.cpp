/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>

#include <osquery/core/core.h>
#include <osquery/core/tables.h>
#include <osquery/filesystem/filesystem.h>
#include <osquery/logger/logger.h>
#include <osquery/sql/sqlite_util.h>
#include <osquery/tables/system/system_utils.h>
#include <osquery/utils/conversions/tryto.h>
#include <osquery/utils/conversions/windows/strings.h>
#include <osquery/utils/system/env.h>
#include <osquery/utils/system/system.h>

namespace fs = boost::filesystem;

namespace osquery {
namespace tables {

const std::string kWingetUninstallKey =
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
const std::string kWingetWow64UninstallKey =
    "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
const std::string kWingetFamiliesKey =
    "Software\\Classes\\Local "
    "Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\Reposi"
    "tory\\Families\\";

// winget's PackageVersionMetadata values, persisted in manifest_metadata
const int kWingetInstalledLocale = 6;
const int kWingetTrackingWriteTime = 7;
const int kWingetInstalledArchitecture = 8;
const int kWingetPinnedState = 9;

// winget keeps one tracking catalog per source under its local state, which
// is the App Installer package folder or, when winget runs unpackaged, its
// own folder in the profile
const std::vector<std::string> kWingetTrackingCatalogs = {
    "AppData/Local/Packages/Microsoft.DesktopAppInstaller_8wekyb3d8bbwe/"
    "LocalState/%/installed.db",
    "AppData/Local/Microsoft/WinGet/State/%/installed.db",
};

struct WingetRecord {
  std::string id;
  std::string name;
  std::string moniker;
  std::string version;
  std::map<int, std::string> metadata;
  std::vector<std::string> productCodes;
  std::vector<std::string> upgradeCodes;
  std::vector<std::string> familyNames;
};

bool wingetKeyExists(HKEY root, const std::string& path) {
  HKEY key = nullptr;
  if (RegOpenKeyExW(root,
                    stringToWstring(path).c_str(),
                    0,
                    KEY_READ | KEY_WOW64_64KEY,
                    &key) != ERROR_SUCCESS) {
    return false;
  }
  RegCloseKey(key);
  return true;
}

// winget keeps a record after it uninstalls the package, so a record counts
// only while one of its Uninstall keys or package families is present. A user
// scope install is visible only while the owner's hive is loaded, that is
// while the owner is logged on.
bool wingetIsInstalled(const WingetRecord& record, const std::string& sid) {
  const bool hiveLoaded = !sid.empty() && wingetKeyExists(HKEY_USERS, sid);
  for (auto code : record.productCodes) {
    boost::algorithm::trim(code);
    if (code.empty()) {
      continue;
    }
    if (wingetKeyExists(HKEY_LOCAL_MACHINE, kWingetUninstallKey + code) ||
        wingetKeyExists(HKEY_LOCAL_MACHINE, kWingetWow64UninstallKey + code) ||
        (hiveLoaded && wingetKeyExists(HKEY_USERS,
                                       sid + "\\" + kWingetUninstallKey +
                                           code))) {
      return true;
    }
  }
  for (const auto& familyName : record.familyNames) {
    if (hiveLoaded &&
        wingetKeyExists(HKEY_USERS,
                        sid + "\\" + kWingetFamiliesKey + familyName)) {
      return true;
    }
  }
  return false;
}

std::string wingetColumnText(sqlite3_stmt* stmt, int column) {
  auto text = sqlite3_column_text(stmt, column);
  return text == nullptr ? "" : reinterpret_cast<const char*>(text);
}

bool wingetQuery(sqlite3* db,
                 const std::string& query,
                 const std::function<void(sqlite3_stmt*)>& onRow) {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return false;
  }
  int rc = SQLITE_OK;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
    onRow(stmt);
  }
  sqlite3_finalize(stmt);
  return rc == SQLITE_DONE;
}

std::string wingetSourceName(const std::string& folder) {
  if (folder == "Microsoft.Winget.Source_8wekyb3d8bbwe") {
    return "winget";
  }
  if (folder == "StoreEdgeFD") {
    return "msstore";
  }
  return folder;
}

std::string wingetMetadata(const WingetRecord& record, int metadata) {
  auto value = record.metadata.find(metadata);
  return value == record.metadata.end() ? "" : value->second;
}

int64_t wingetWriteTime(const WingetRecord& record) {
  return tryTo<int64_t>(wingetMetadata(record, kWingetTrackingWriteTime))
      .takeOr(int64_t{0});
}

void genWingetTrackingCatalog(const std::string& path,
                              const std::string& uid,
                              const std::string& sid,
                              QueryData& results) {
  sqlite3* db = nullptr;
  auto rc = sqlite3_open_v2(
      path.c_str(),
      &db,
      SQLITE_OPEN_READONLY | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_NOMUTEX,
      nullptr);
  if (rc != SQLITE_OK || db == nullptr) {
    VLOG(1) << "Cannot open winget tracking catalog " << path << ": "
            << getStringForSQLiteReturnCode(rc);
    sqlite3_close(db);
    return;
  }

  std::set<std::string> tables;
  std::string majorVersion;
  bool read =
      wingetQuery(db,
                  "SELECT name FROM sqlite_master WHERE type = 'table'",
                  [&tables](sqlite3_stmt* stmt) {
                    tables.insert(wingetColumnText(stmt, 0));
                  }) &&
      tables.count("metadata") > 0 && tables.count("manifest") > 0 &&
      wingetQuery(db,
                  "SELECT value FROM metadata WHERE name = 'majorVersion'",
                  [&majorVersion](sqlite3_stmt* stmt) {
                    majorVersion = wingetColumnText(stmt, 0);
                  }) &&
      majorVersion == "1";

  // winget writes one record per installed version and keeps them after an
  // uninstall
  std::map<int64_t, WingetRecord> records;
  read = read &&
         wingetQuery(
             db,
             "SELECT m.rowid, i.id, n.name, IFNULL(o.moniker, ''), v.version "
             "FROM manifest m JOIN ids i ON i.rowid = m.id "
             "JOIN names n ON n.rowid = m.name "
             "JOIN versions v ON v.rowid = m.version "
             "LEFT JOIN monikers o ON o.rowid = m.moniker",
             [&records](sqlite3_stmt* stmt) {
               auto& record = records[sqlite3_column_int64(stmt, 0)];
               record.id = wingetColumnText(stmt, 1);
               record.name = wingetColumnText(stmt, 2);
               record.moniker = wingetColumnText(stmt, 3);
               record.version = wingetColumnText(stmt, 4);
             });

  // Tables added by later schema versions are read when present
  if (read && tables.count("manifest_metadata") > 0) {
    read = wingetQuery(
        db,
        "SELECT manifest, metadata, value FROM manifest_metadata",
        [&records](sqlite3_stmt* stmt) {
          auto record = records.find(sqlite3_column_int64(stmt, 0));
          if (record != records.end()) {
            record->second.metadata[sqlite3_column_int(stmt, 1)] =
                wingetColumnText(stmt, 2);
          }
        });
  }
  auto readList = [&](const std::string& column,
                      std::vector<std::string> WingetRecord::*list) {
    const auto table = column + "s";
    if (!read || tables.count(table) == 0 ||
        tables.count(table + "_map") == 0) {
      return;
    }
    read = wingetQuery(db,
                       "SELECT x.manifest, v." + column + " FROM " + table +
                           "_map x JOIN " + table + " v ON v.rowid = x." +
                           column,
                       [&records, list](sqlite3_stmt* stmt) {
                         auto record =
                             records.find(sqlite3_column_int64(stmt, 0));
                         if (record != records.end()) {
                           (record->second.*list)
                               .push_back(wingetColumnText(stmt, 1));
                         }
                       });
  };
  readList("productcode", &WingetRecord::productCodes);
  readList("upgradecode", &WingetRecord::upgradeCodes);
  readList("pfn", &WingetRecord::familyNames);
  sqlite3_close(db);

  if (!read) {
    VLOG(1) << "Skipping winget tracking catalog " << path;
    return;
  }

  // The latest record of a package is the one winget wrote last
  std::map<std::string, std::pair<int64_t, const WingetRecord*>> latest;
  for (const auto& record : records) {
    auto& current = latest[record.second.id];
    auto written = wingetWriteTime(record.second);
    if (current.second == nullptr || written >= current.first) {
      current = {written, &record.second};
    }
  }

  const auto source =
      wingetSourceName(fs::path(path).parent_path().filename().string());
  for (const auto& package : latest) {
    const auto& record = *package.second.second;
    if (!wingetIsInstalled(record, sid)) {
      continue;
    }

    Row r;
    r["id"] = record.id;
    r["name"] = record.name;
    r["version"] = record.version;
    r["moniker"] = record.moniker;
    r["source"] = source;
    r["installed_at"] = BIGINT(package.second.first);
    r["installed_architecture"] =
        wingetMetadata(record, kWingetInstalledArchitecture);
    r["installed_locale"] = wingetMetadata(record, kWingetInstalledLocale);
    r["pinned_state"] = wingetMetadata(record, kWingetPinnedState);
    r["product_codes"] = boost::algorithm::join(record.productCodes, ",");
    r["upgrade_codes"] = boost::algorithm::join(record.upgradeCodes, ",");
    r["package_family_names"] =
        boost::algorithm::join(record.familyNames, ",");
    r["uid"] = uid;
    r["path"] = path;
    results.push_back(r);
  }
}

QueryData genWingetPackages(QueryContext& context) {
  QueryData results;
  for (const auto& user : usersFromContext(context)) {
    auto directory = user.find("directory");
    auto uid = user.find("uid");
    auto sid = user.find("uuid");
    if (directory == user.end() || directory->second.empty() ||
        uid == user.end() || sid == user.end()) {
      continue;
    }

    // Service accounts list their profile as %systemroot%\...
    auto profile = expandEnvString(directory->second);
    if (!profile) {
      continue;
    }

    for (const auto& pattern : kWingetTrackingCatalogs) {
      std::vector<std::string> catalogs;
      resolveFilePattern(fs::path(*profile) / pattern, catalogs, GLOB_FILES);
      for (const auto& catalog : catalogs) {
        genWingetTrackingCatalog(catalog, uid->second, sid->second, results);
      }
    }
  }
  return results;
}

} // namespace tables
} // namespace osquery
