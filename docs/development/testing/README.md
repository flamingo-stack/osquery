# Testing Guide

osquery uses **Google Test** (GTest) and **Google Mock** (GMock) for unit and integration testing. Tests are organized alongside the source code in `tests/` subdirectories within each module.

---

## Test Structure and Organization

```text
osquery/
├── core/
│   ├── tests/
│   │   ├── flags_tests.cpp
│   │   ├── query_tests.cpp
│   │   ├── system_test.cpp
│   │   └── tables_tests.cpp
├── sql/
│   └── tests/
│       ├── sql.cpp
│       └── virtual_table.cpp
├── events/
│   └── tests/
│       ├── events_tests.cpp
│       └── linux/
│           ├── inotify_tests.cpp
│           └── bpf/
├── config/
│   └── tests/
│       ├── config_tests.cpp
│       └── packs.cpp
├── database/
│   └── tests/
├── distributed/
│   └── tests/
└── extensions/
    └── tests/

tests/
└── integration/
    └── tables/     # Integration tests for all virtual tables
        ├── processes.cpp
        ├── users.cpp
        ├── listening_ports.cpp
        └── ...

plugins/
└── */tests/        # Plugin-level tests
```

### Test Categories

| Category | Location | Description |
|---|---|---|
| **Unit Tests** | `osquery/*/tests/` | Fast, isolated, no OS dependencies |
| **Integration Tests** | `tests/integration/tables/` | Live table queries against the real OS |
| **Benchmark Tests** | `osquery/*/benchmarks/` | Performance measurements |
| **Extension Tests** | `osquery/extensions/tests/` | IPC and Thrift extension round-trips |
| **Plugin Tests** | `plugins/*/tests/` | Logger, config, and database plugins |

---

## Building Tests

Tests must be explicitly enabled at CMake configure time:

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON

cmake --build build --parallel $(nproc)
```

---

## Running Tests

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Tests in Parallel

```bash
cd build
ctest --output-on-failure --parallel $(nproc)
```

### Run a Specific Test Suite

```bash
cd build

# Run core tests
ctest -R "osquery_core_tests" --output-on-failure

# Run SQL tests
ctest -R "osquery_sql_tests" --output-on-failure

# Run config tests
ctest -R "osquery_config_tests" --output-on-failure

# Run events tests
ctest -R "osquery_events_tests" --output-on-failure

# Run integration table tests
ctest -R "integration" --output-on-failure
```

### Run Tests by Keyword

```bash
cd build
ctest -R "tls" --output-on-failure
ctest -R "database" --output-on-failure
ctest -R "extension" --output-on-failure
```

### Verbose Test Output

```bash
cd build
ctest -V -R "osquery_core_tests"
```

---

## Writing New Tests

### Unit Test Template

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "osquery/my_module/my_class.h"

namespace osquery {

class MyClassTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Per-test setup
  }

  void TearDown() override {
    // Per-test cleanup
  }
};

TEST_F(MyClassTest, BasicOperation) {
  MyClass obj;
  EXPECT_EQ(obj.doSomething(), expected_value);
}

TEST_F(MyClassTest, HandlesEdgeCase) {
  MyClass obj;
  EXPECT_THROW(obj.doSomethingInvalid(), std::runtime_error);
}

} // namespace osquery
```

### Virtual Table Integration Test Template

```cpp
#include "tests/integration/tables/helper.h"

namespace osquery {
namespace table_tests {

class MyTableTest : public testing::Test {};

TEST_F(MyTableTest, test_sanity) {
  // Validate table schema and basic query
  auto const data = execute_query("SELECT * FROM my_table;");
  ASSERT_GT(data.size(), 0ul);

  // Validate specific column presence and types
  ValidateSchema(
    data,
    {
      {"column_one",   BIGINT_TYPE,  CHECK_GT, 0, CHECK_LT, INT_MAX},
      {"column_two",   TEXT_TYPE,    CHECK_NON_EMPTY},
    }
  );
}

} // namespace table_tests
} // namespace osquery
```

### Using GMock for Dependencies

```cpp
#include <gmock/gmock.h>
#include "osquery/database/database.h"

class MockDatabase : public IDatabaseInterface {
 public:
  MOCK_METHOD(Status, putBatch,
    (const std::string& domain,
     const DatabaseStringValueList& data),
    (override));

  MOCK_METHOD(Status, get,
    (const std::string& domain,
     const std::string& key,
     std::string& value),
    (const, override));
};

TEST(MyServiceTest, PersistsData) {
  MockDatabase db;
  EXPECT_CALL(db, putBatch(testing::_, testing::_))
    .Times(1)
    .WillOnce(testing::Return(Status::success()));

  MyService service(db);
  auto result = service.persist("key", "value");
  EXPECT_TRUE(result.ok());
}
```

---

## Test Helpers

The integration test suite includes shared helpers in `tests/integration/tables/helper.h`:

| Helper | Purpose |
|---|---|
| `execute_query(sql)` | Execute SQL and return rows |
| `ValidateSchema(data, validators)` | Assert column types and value ranges |
| `CHECK_GT`, `CHECK_LT` | Numeric range validators |
| `CHECK_NON_EMPTY` | String non-emptiness check |

---

## Benchmark Tests

Benchmarks measure performance of hot paths:

```bash
# Build benchmarks
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DOSQUERY_BUILD_BENCHMARKS=ON
cmake --build build --parallel $(nproc)

# Run SQL benchmarks
./build/osquery/sql/benchmarks/sql_benchmarks

# Run events benchmarks
./build/osquery/events/benchmarks/events_benchmarks
```

---

## Coverage Requirements

While no mandatory coverage threshold is enforced in CI currently, the project encourages:

- **Unit tests** for all new core module functionality
- **Integration tests** for every new virtual table
- **Error path coverage** for all error handling branches
- **Mock-based tests** for components with external dependencies (network, database)

When contributing new virtual tables, a corresponding integration test in `tests/integration/tables/` is expected.

---

## Platform-Specific Tests

Some tests only run on specific platforms:

```cpp
// Linux-only test
#ifdef __linux__
TEST_F(AuditTest, LinuxAuditPublisher) {
  // Linux-specific audit test
}
#endif

// macOS-only test
#ifdef __APPLE__
TEST_F(FSEventsTest, FseventsPublisher) {
  // macOS FSEvents test
}
#endif
```

The CMake build system automatically includes platform-appropriate test targets.

---

## Continuous Integration

Tests run automatically in CI on every pull request. The CI pipeline:

1. Builds with `-DOSQUERY_BUILD_TESTS=ON`
2. Runs `ctest --output-on-failure`
3. Enforces `clang-format` compliance
4. Checks copyright headers (via `tools/ci/scripts/check_copyright_headers.py`)

All tests must pass before a PR can be merged.
