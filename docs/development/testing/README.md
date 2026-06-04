# Testing Guide

This document describes the test structure, how to run tests, and how to write new tests for osquery with the OpenFrame integration.

---

## Test Structure and Organization

osquery uses **Google Test (gtest)** as its primary unit testing framework. Tests are co-located with the source modules they test:

```text
osquery/
├── core/
│   ├── core.cpp
│   └── tests/
│       ├── flags_tests.cpp
│       ├── process_tests.cpp
│       ├── query_tests.cpp
│       └── tables_tests.cpp
├── sql/
│   ├── sql.cpp
│   └── tests/
│       ├── sql.cpp
│       └── sqlite_util_tests.cpp
├── events/
│   ├── eventer.cpp
│   └── tests/
│       ├── events_tests.cpp
│       └── linux/
│           ├── audit_tests.cpp
│           └── inotify_tests.cpp
└── database/
    ├── database.cpp
    └── tests/
        └── database.cpp
```

Integration tests live in a separate top-level directory:

```text
tests/integration/tables/
├── processes.cpp
├── listening_ports.cpp
├── system_info.cpp
└── ... (one file per virtual table)
```

---

## Running Tests

### Run All Tests

```bash
# From the build directory
cd build
ctest --output-on-failure

# With parallel execution
ctest --output-on-failure -j$(nproc)
```

### Run a Specific Test Suite

```bash
# Run a specific test binary directly
./build/osquery_sql_tests
./build/osquery_core_tests
./build/osquery_events_tests
./build/osquery_database_tests
./build/osquery_filesystem_tests
```

### Run a Specific Test Case

```bash
# Google Test filter syntax
./build/osquery_sql_tests --gtest_filter="SQLTests.*"
./build/osquery_core_tests --gtest_filter="FlagsTests.testCustomFlags"

# Run tests matching a pattern
./build/osquery_sql_tests --gtest_filter="*SQLiteEncoding*"
```

### Run Tests with Verbose Output

```bash
./build/osquery_sql_tests --gtest_verbose=1
# or
ctest --output-on-failure --verbose
```

---

## Building Test Targets

```bash
# Configure with tests enabled
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON

# Build all test targets
cmake --build build --target all -j$(nproc)

# Build a specific test target
cmake --build build --target osquery_sql_tests -j$(nproc)
cmake --build build --target osquery_core_tests -j$(nproc)
cmake --build build --target osquery_events_tests -j$(nproc)
```

---

## Writing New Tests

### Unit Test — Basic Structure

Create a new test file co-located with the source:

```cpp
// osquery/sql/tests/my_feature_tests.cpp

#include <gtest/gtest.h>
#include <osquery/sql/sql.h>
#include <osquery/database/database.h>

namespace osquery {

class MyFeatureTests : public testing::Test {
 protected:
  void SetUp() override {
    // Use ephemeral database for tests — never persistent
    setDatabaseAllowOpen();
    initDatabasePlugin();
  }

  void TearDown() override {
    shutdownDatabase();
  }
};

TEST_F(MyFeatureTests, testBasicQuery) {
  auto results = SQL::selectAllFrom("osquery_info");
  EXPECT_GT(results.size(), 0U);
}

TEST_F(MyFeatureTests, testEmptyResult) {
  auto results = SQL::selectAllFrom("processes", "pid", EQUALS, "999999999");
  EXPECT_EQ(results.size(), 0U);
}

} // namespace osquery
```

### Integration Test — Virtual Table

Integration tests verify a virtual table returns expected results on the real OS:

```cpp
// tests/integration/tables/my_new_table.cpp

#include <gtest/gtest.h>
#include "helper.h"

namespace osquery::table_tests {

class MyNewTableTest : public testing::Test {};

TEST_F(MyNewTableTest, test_implementation) {
  auto const data = execute_query("SELECT * FROM my_new_table;");
  ASSERT_GE(data.size(), 0U);
  ValidateSchema(data, {
    {"column_a", INTEGER_TYPE},
    {"column_b", TEXT_TYPE},
  });
}

} // namespace osquery::table_tests
```

### Test with Ephemeral Database

Most tests need a database. Always use the ephemeral backend in tests:

```cpp
#include <osquery/database/database.h>

class DatabaseTest : public testing::Test {
 protected:
  void SetUp() override {
    // Force ephemeral (in-memory) database for test isolation
    FLAGS_disable_database = true;
    setDatabaseAllowOpen();
    initDatabasePlugin();
  }

  void TearDown() override {
    shutdownDatabase();
  }
};
```

---

## Test Coverage Areas

| Module | Test Binary | Key Test Files |
|--------|-------------|---------------|
| Core Flags | `osquery_core_tests` | `core/tests/flags_tests.cpp` |
| SQL Engine | `osquery_sql_tests` | `sql/tests/sql.cpp` |
| Virtual Tables | `osquery_sql_tests` | `sql/tests/virtual_table.cpp` |
| Events | `osquery_events_tests` | `events/tests/events_tests.cpp` |
| Database | `osquery_database_tests` | `database/tests/database.cpp` |
| Config | `osquery_config_tests` | `config/tests/config_tests.cpp` |
| Hashing | `osquery_hashing_tests` | `hashing/tests/hashing.cpp` |
| Filesystem | `osquery_filesystem_tests` | `filesystem/tests/filesystem.cpp` |
| Extensions | `osquery_extensions_tests` | `extensions/tests/extensions.cpp` |
| Distributed | `osquery_distributed_tests` | `distributed/tests/distributed_tests.cpp` |

---

## Benchmarks

Performance-sensitive modules have benchmark suites using **Google Benchmark**:

```bash
# Build benchmarks
cmake -S . -B build \
  -DOSQUERY_BUILD_BENCHMARKS=ON \
  -DCMAKE_BUILD_TYPE=Release

# Run SQL benchmarks
./build/osquery_sql_benchmarks

# Run events benchmarks
./build/osquery_events_benchmarks

# Run database benchmarks
./build/osquery_database_benchmarks
```

---

## Testing the OpenFrame Integration

The OpenFrame modules (`openframe/`) should be tested with mock token files:

```bash
# Create a test token file
echo "Bearer test-token-value" > /tmp/test_openframe_token
chmod 600 /tmp/test_openframe_token

# Run osqueryi with OpenFrame mode to verify token loading
./build/osqueryi \
  --openframe_mode=true \
  --openframe_token_path=/tmp/test_openframe_token \
  --verbose
```

---

## CI Test Matrix

Tests are run across the following configurations:

| Platform | Compiler | Build Type |
|----------|----------|------------|
| Linux x86_64 | GCC 11 | Debug + Release |
| Linux aarch64 | GCC 11 | Debug |
| macOS x86_64 | AppleClang | Debug + Release |
| macOS aarch64 | AppleClang | Debug + Release |
| Windows x86_64 | MSVC 2022 | Debug + Release |

---

## Common Test Failures and Fixes

| Error | Likely Cause | Fix |
|-------|-------------|-----|
| `database not initialized` | Missing `initDatabasePlugin()` in `SetUp()` | Add database init to test fixture |
| `permission denied` on `/proc` | Running tests as non-root | Some Linux tables require root for full data |
| `table not found` | Virtual table not registered in test binary | Ensure the table plugin is linked |
| `gtest: test binary not found` | `OSQUERY_BUILD_TESTS=OFF` | Reconfigure CMake with `OSQUERY_BUILD_TESTS=ON` |
