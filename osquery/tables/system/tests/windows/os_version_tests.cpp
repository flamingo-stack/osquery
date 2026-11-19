/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

#include <gtest/gtest.h>

#include <osquery/tables/system/windows/os_version.h>

namespace osquery {
namespace tables {

class OSVersionTests : public testing::Test {
 protected:
  std::map<std::string, std::string> windows11Pro() {
    return {
        {"ProductName", "Windows 10 Pro"},
        {"CurrentMajorVersionNumber", "10"},
        {"CurrentMinorVersionNumber", "0"},
        {"CurrentVersion", "6.3"},
        {"CurrentBuildNumber", "22631"},
        {"UBR", "4460"},
        {"InstallDate", "1700000000"},
    };
  }
};

TEST_F(OSVersionTests, test_windows_11_from_registry) {
  auto results = genOSVersionFromRegistry(windows11Pro(), "64-bit");

  ASSERT_EQ(results.size(), 1U);
  const auto& r = results[0];
  EXPECT_EQ(r.at("name"), "Microsoft Windows 11 Pro");
  EXPECT_EQ(r.at("codename"), "Microsoft Windows 11 Pro");
  EXPECT_EQ(r.at("version"), "10.0.22631");
  EXPECT_EQ(r.at("major"), "10");
  EXPECT_EQ(r.at("minor"), "0");
  EXPECT_EQ(r.at("build"), "22631");
  EXPECT_EQ(r.at("revision"), "4460");
  EXPECT_EQ(r.at("install_date"), "1700000000");
  EXPECT_EQ(r.at("arch"), "64-bit");
  EXPECT_EQ(r.at("platform"), "windows");
  EXPECT_EQ(r.at("platform_like"), "windows");
}

TEST_F(OSVersionTests, test_windows_10_keeps_product_name) {
  auto currentVersion = windows11Pro();
  currentVersion["CurrentBuildNumber"] = "19045";

  auto results = genOSVersionFromRegistry(currentVersion, "64-bit");

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].at("name"), "Microsoft Windows 10 Pro");
  EXPECT_EQ(results[0].at("version"), "10.0.19045");
}

TEST_F(OSVersionTests, test_server_keeps_product_name) {
  auto currentVersion = windows11Pro();
  currentVersion["ProductName"] = "Windows Server 2025 Datacenter";
  currentVersion["CurrentBuildNumber"] = "26100";

  auto results = genOSVersionFromRegistry(currentVersion, "64-bit");

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].at("name"), "Microsoft Windows Server 2025 Datacenter");
}

TEST_F(OSVersionTests, test_version_from_current_version_without_numbers) {
  auto currentVersion = windows11Pro();
  currentVersion.erase("CurrentMajorVersionNumber");
  currentVersion.erase("CurrentMinorVersionNumber");
  currentVersion["ProductName"] = "Windows Server 2012 R2 Standard";
  currentVersion["CurrentBuildNumber"] = "9600";

  auto results = genOSVersionFromRegistry(currentVersion, "64-bit");

  ASSERT_EQ(results.size(), 1U);
  EXPECT_EQ(results[0].at("version"), "6.3.9600");
  EXPECT_EQ(results[0].at("major"), "6");
  EXPECT_EQ(results[0].at("minor"), "3");
}

TEST_F(OSVersionTests, test_no_row_without_product_name_or_build) {
  for (const auto& missing : {"ProductName", "CurrentBuildNumber"}) {
    auto currentVersion = windows11Pro();
    currentVersion.erase(missing);

    EXPECT_TRUE(genOSVersionFromRegistry(currentVersion, "64-bit").empty())
        << missing;
  }
}

TEST_F(OSVersionTests, test_no_row_without_version) {
  auto currentVersion = windows11Pro();
  currentVersion.erase("CurrentMajorVersionNumber");
  currentVersion.erase("CurrentVersion");

  EXPECT_TRUE(genOSVersionFromRegistry(currentVersion, "64-bit").empty());
}

} // namespace tables
} // namespace osquery
