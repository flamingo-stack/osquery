/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <osquery/core/tables.h>

#include <osquery/core/windows/wmi.h>
#include <osquery/logger/logger.h>
#include <osquery/tables/system/windows/os_version.h>
#include <osquery/utils/conversions/split.h>
#include <osquery/utils/conversions/tryto.h>
#include <osquery/utils/conversions/windows/strings.h>

#include "osquery/tables/system/windows/registry.h"
namespace osquery {
namespace tables {

namespace {

const std::string kCurrentVersionKey =
    "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

const int kWindows11FirstBuild = 22000;

std::string getNativeArchitecture() {
  SYSTEM_INFO info;
  GetNativeSystemInfo(&info);
  switch (info.wProcessorArchitecture) {
  case PROCESSOR_ARCHITECTURE_AMD64:
    return "64-bit";
  case PROCESSOR_ARCHITECTURE_INTEL:
    return "32-bit";
  case PROCESSOR_ARCHITECTURE_ARM64:
    return "ARM 64-bit Processor";
  default:
    return "";
  }
}

QueryData genOSVersionFallback(
    const std::map<std::string, std::string>& currentVersion) {
  auto results =
      genOSVersionFromRegistry(currentVersion, getNativeArchitecture());
  if (results.empty()) {
    LOG(WARNING) << "No OS version in " << kCurrentVersionKey;
  }
  return results;
}

} // namespace

QueryData genOSVersionFromRegistry(
    const std::map<std::string, std::string>& currentVersion,
    const std::string& arch) {
  auto value = [&currentVersion](const std::string& name) {
    const auto it = currentVersion.find(name);
    return it == currentVersion.end() ? std::string{} : it->second;
  };

  auto productName = value("ProductName");
  const auto build = value("CurrentBuildNumber");
  auto major = value("CurrentMajorVersionNumber");
  auto minor = value("CurrentMinorVersionNumber");
  if (major.empty()) {
    const auto legacyVersion = osquery::split(value("CurrentVersion"), ".");
    if (legacyVersion.size() == 2) {
      major = legacyVersion[0];
      minor = legacyVersion[1];
    }
  }
  if (productName.empty() || build.empty() || major.empty()) {
    return {};
  }

  const std::string kWindows10 = "Windows 10";
  if (productName.rfind(kWindows10, 0) == 0 &&
      tryTo<int>(build).takeOr(0) >= kWindows11FirstBuild) {
    productName.replace(0, kWindows10.size(), "Windows 11");
  }

  Row r;
  r["name"] = "Microsoft " + productName;
  r["codename"] = r["name"];
  r["major"] = major;
  r["minor"] = minor;
  r["build"] = build;
  r["version"] = major + "." + minor + "." + build;
  r["arch"] = arch;
  r["install_date"] = value("InstallDate");
  r["revision"] = value("UBR");
  r["platform"] = "windows";
  r["platform_like"] = "windows";
  return {r};
}

QueryData genOSVersion(QueryContext& context) {
  Row r;
  std::string version_string;

  QueryData regResults;
  queryKey(kCurrentVersionKey, regResults);
  std::map<std::string, std::string> currentVersion;
  for (const auto& aKey : regResults) {
    currentVersion[aKey.at("name")] = aKey.at("data");
  }

  const std::string kWmiQuery =
      "SELECT CAPTION,VERSION,INSTALLDATE,OSARCHITECTURE FROM "
      "Win32_OperatingSystem";

  const auto wmiRequest = WmiRequest::CreateWmiRequest(kWmiQuery);

  if (!wmiRequest) {
    LOG(WARNING) << wmiRequest.getError().getMessage();
    return genOSVersionFallback(currentVersion);
  }

  const std::vector<WmiResultItem>& wmiResults = wmiRequest->results();

  if (wmiResults.empty()) {
    const auto enumResult = wmiRequest->getEnumerationResult();
    if (FAILED(enumResult)) {
      LOG(WARNING) << "WMI query on Win32_OperatingSystem failed while "
                      "reading results: 0x"
                   << std::hex << enumResult;
    } else {
      LOG(WARNING) << "WMI query on Win32_OperatingSystem returned no results";
    }
    return genOSVersionFallback(currentVersion);
  }

  std::string osName;
  wmiResults[0].GetString("Caption", osName);
  r["name"] = osName;
  r["codename"] = osName;

  std::string cimInstallDate{""};
  wmiResults[0].GetString("InstallDate", cimInstallDate);
  r["install_date"] = BIGINT(cimDatetimeToUnixtime(cimInstallDate));

  wmiResults[0].GetString("Version", version_string);
  auto version = osquery::split(version_string, ".");

  switch (version.size()) {
  case 3:
    r["build"] = SQL_TEXT(version[2]);
  case 2:
    r["minor"] = INTEGER(version[1]);
  case 1:
    r["major"] = INTEGER(version[0]);
    break;
  default:
    break;
  }

  wmiResults[0].GetString("OSArchitecture", r["arch"]);

  r["platform"] = "windows";
  r["platform_like"] = "windows";
  r["version"] = r["major"] + "." + r["minor"] + "." + r["build"];

  const auto ubr = currentVersion.find("UBR");
  r["revision"] =
      INTEGER(ubr == currentVersion.end() ? std::string{} : ubr->second);

  return {r};
}
} // namespace tables
} // namespace osquery
