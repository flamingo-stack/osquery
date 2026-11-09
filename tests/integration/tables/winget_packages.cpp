/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

// Sanity check integration test for winget_packages
// Spec file: specs/windows/winget_packages.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {
namespace table_tests {

class WingetPackagesTest : public testing::Test {
 protected:
  void SetUp() override {
    setUpEnvironment();
  }
};

TEST_F(WingetPackagesTest, test_sanity) {
  auto const data = execute_query("select * from winget_packages");

  ValidationMap row_map = {
      {"id", NonEmptyString},
      {"name", NormalType},
      {"version", NormalType},
      {"moniker", NormalType},
      {"source", NonEmptyString},
      {"installed_at", NonNegativeInt},
      {"installed_architecture", NormalType},
      {"installed_locale", NormalType},
      {"pinned_state", NormalType},
      {"product_codes", NormalType},
      {"upgrade_codes", NormalType},
      {"package_family_names", NormalType},
      {"uid", IntType},
      {"path", NonEmptyString},
  };
  validate_rows(data, row_map);
}

} // namespace table_tests
} // namespace osquery
