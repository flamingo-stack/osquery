/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed as defined by the LICENSE file found in the
 * root directory of this source tree.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR GPL-2.0-only)
 */

// Sanity check integration test for kernel_modules
// Spec file: specs/linux/kernel_modules.table

#include <osquery/tests/integration/tables/helper.h>

namespace osquery {
namespace table_tests {

class KernelKeys : public testing::Test {
 protected:
  void SetUp() override {
    setUpEnvironment();
  }
};

TEST_F(KernelKeys, test_sanity) {
  QueryData data = execute_query("select * from kernel_keys");
  ValidationMap row_map = {
      {"serial_number", NonEmptyString},
      {"description", NormalType},
  };
  validate_rows(data, row_map);
}

} // namespace table_tests
} // namespace osquery

