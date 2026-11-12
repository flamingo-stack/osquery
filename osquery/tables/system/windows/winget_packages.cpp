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
#include <memory>
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
// Relative to the user's classes, which UsrClass.dat holds
const std::string kWingetFamiliesKey =
    "Local "
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

// The Offline Registry Library reads a hive file without loading it into the
// registry. RegLoadAppKeyW refuses user profile hives with ERROR_BADDB.
struct WingetOffreg {
  using Handle = void*;
  DWORD(WINAPI* openHive)(PCWSTR, Handle*) = nullptr;
  DWORD(WINAPI* openKey)(Handle, PCWSTR, Handle*) = nullptr;
  DWORD(WINAPI* closeKey)(Handle) = nullptr;
  DWORD(WINAPI* closeHive)(Handle) = nullptr;
};

const WingetOffreg* wingetOffreg() {
  static const auto api = []() -> std::unique_ptr<WingetOffreg> {
    auto lib =
        LoadLibraryExW(L"offreg.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (lib == nullptr) {
      VLOG(1) << "offreg.dll is not available: " << GetLastError();
      return nullptr;
    }
    auto api = std::make_unique<WingetOffreg>();
    api->openHive = reinterpret_cast<decltype(api->openHive)>(
        GetProcAddress(lib, "OROpenHive"));
    api->openKey = reinterpret_cast<decltype(api->openKey)>(
        GetProcAddress(lib, "OROpenKey"));
    api->closeKey = reinterpret_cast<decltype(api->closeKey)>(
        GetProcAddress(lib, "ORCloseKey"));
    api->closeHive = reinterpret_cast<decltype(api->closeHive)>(
        GetProcAddress(lib, "ORCloseHive"));
    if (api->openHive == nullptr || api->openKey == nullptr ||
        api->closeKey == nullptr || api->closeHive == nullptr) {
      VLOG(1) << "offreg.dll does not export the hive functions";
      FreeLibrary(lib);
      return nullptr;
    }
    return api;
  }();
  return api.get();
}

// The registry of one user. Windows loads it under HKEY_USERS only while the
// user is logged on; otherwise a copy of the hive file is read offline, so the
// original stays free for the user's next logon.
class WingetUserHive {
 public:
  WingetUserHive(const std::string& sid, const fs::path& profile)
      : sid_(sid),
        profile_(profile),
        loaded_(!sid.empty() && wingetKeyExists(HKEY_USERS, sid)) {}

  ~WingetUserHive() {
    for (auto hive : {user_, classes_}) {
      if (hive != nullptr) {
        wingetOffreg()->closeHive(hive);
      }
    }
    if (!copies_.empty()) {
      boost::system::error_code ec;
      fs::remove_all(copies_, ec);
    }
  }

  WingetUserHive(const WingetUserHive&) = delete;
  WingetUserHive& operator=(const WingetUserHive&) = delete;

  bool userKeyExists(const std::string& path) {
    if (loaded_) {
      return wingetKeyExists(HKEY_USERS, sid_ + "\\" + path);
    }
    if (!userTried_) {
      userTried_ = true;
      user_ = openCopy(profile_ / "NTUSER.DAT");
    }
    return user_ != nullptr && offlineKeyExists(user_, path);
  }

  bool classesKeyExists(const std::string& path) {
    if (loaded_) {
      return wingetKeyExists(HKEY_USERS,
                             sid_ + "\\Software\\Classes\\" + path);
    }
    if (!classesTried_) {
      classesTried_ = true;
      classes_ =
          openCopy(profile_ / "AppData/Local/Microsoft/Windows/UsrClass.dat");
    }
    return classes_ != nullptr && offlineKeyExists(classes_, path);
  }

 private:
  static bool offlineKeyExists(WingetOffreg::Handle hive,
                               const std::string& path) {
    WingetOffreg::Handle key = nullptr;
    if (wingetOffreg()->openKey(hive, stringToWstring(path).c_str(), &key) !=
        ERROR_SUCCESS) {
      return false;
    }
    wingetOffreg()->closeKey(key);
    return true;
  }

  // Windows writes the hive file when it unloads the hive at sign-out, so the
  // file alone is current; its transaction logs are not needed
  WingetOffreg::Handle openCopy(const fs::path& hive) {
    if (wingetOffreg() == nullptr) {
      return nullptr;
    }

    boost::system::error_code ec;
    if (copies_.empty()) {
      auto copies = fs::temp_directory_path(ec) /
                    fs::unique_path("osquery-winget-%%%%-%%%%-%%%%");
      if (ec || !fs::create_directories(copies, ec)) {
        VLOG(1) << "Cannot create a folder for registry hive copies";
        return nullptr;
      }
      copies_ = copies;
    }

    const auto copy = copies_ / hive.filename();
    fs::copy_file(hive, copy, ec);
    if (ec) {
      VLOG(1) << "Cannot copy registry hive " << hive.string() << ": "
              << ec.message();
      return nullptr;
    }

    WingetOffreg::Handle opened = nullptr;
    auto rc = wingetOffreg()->openHive(copy.wstring().c_str(), &opened);
    if (rc != ERROR_SUCCESS) {
      VLOG(1) << "Cannot open registry hive " << hive.string() << ": " << rc;
      return nullptr;
    }
    return opened;
  }

  const std::string sid_;
  const fs::path profile_;
  const bool loaded_;
  fs::path copies_;
  WingetOffreg::Handle user_ = nullptr;
  WingetOffreg::Handle classes_ = nullptr;
  bool userTried_ = false;
  bool classesTried_ = false;
};

// winget keeps a record after it uninstalls the package, so a record counts
// only while one of its Uninstall keys or package families is present. The
// owner's hive is read only when the machine keys do not answer.
bool wingetIsInstalled(const WingetRecord& record, WingetUserHive& hive) {
  for (auto code : record.productCodes) {
    boost::algorithm::trim(code);
    if (code.empty()) {
      continue;
    }
    if (wingetKeyExists(HKEY_LOCAL_MACHINE, kWingetUninstallKey + code) ||
        wingetKeyExists(HKEY_LOCAL_MACHINE, kWingetWow64UninstallKey + code) ||
        hive.userKeyExists(kWingetUninstallKey + code)) {
      return true;
    }
  }
  for (const auto& familyName : record.familyNames) {
    if (hive.classesKeyExists(kWingetFamiliesKey + familyName)) {
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
                              WingetUserHive& hive,
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
    if (!wingetIsInstalled(record, hive)) {
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

    WingetUserHive hive(sid->second, *profile);
    for (const auto& pattern : kWingetTrackingCatalogs) {
      std::vector<std::string> catalogs;
      resolveFilePattern(fs::path(*profile) / pattern, catalogs, GLOB_FILES);
      for (const auto& catalog : catalogs) {
        genWingetTrackingCatalog(catalog, uid->second, hive, results);
      }
    }
  }
  return results;
}

} // namespace tables
} // namespace osquery
