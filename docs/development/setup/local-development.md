# Local Development Guide

This guide covers cloning the repository, building osquery locally, running the interactive shell and daemon, and working with the extension SDK.

---

## Clone and Initial Setup

```bash
# Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# Verify the structure
ls -la
```

> osquery bundles almost all of its third-party dependencies under `libraries/cmake/source/`. You do not need to install RocksDB, Thrift, Boost, or OpenSSL separately — CMake fetches and builds them from source.

---

## Configuring the Build

osquery uses **CMake** (minimum 3.21) with **Ninja** as the recommended backend.

### Standard Development Build

```bash
cmake -B build -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Build Type Reference

| Type | Use Case |
|---|---|
| `Debug` | Debugging, sanitizers, full symbols |
| `RelWithDebInfo` | Day-to-day development — fast + debuggable |
| `Release` | Performance testing, packaging |

### Common CMake Options

| Option | Default | Description |
|---|---|---|
| `OSQUERY_BUILD_TESTS` | `ON` | Build unit and integration tests |
| `OSQUERY_BUILD_BPF` | auto-detected | Enable eBPF event publisher (Linux) |
| `OSQUERY_DISABLE_DATABASE_PERF_ISSUE_WORKAROUND` | `OFF` | Database performance workaround |
| `OSQUERY_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer |
| `OSQUERY_ENABLE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer |

### Enabling Sanitizers (for debugging)

```bash
cmake -B build-asan -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_ENABLE_ASAN=ON \
  -DOSQUERY_ENABLE_UBSAN=ON
```

---

## Building Targets

### Build Individual Binaries

```bash
# Interactive shell (fastest to build)
cmake --build build --target osqueryi -j$(nproc)

# Daemon
cmake --build build --target osqueryd -j$(nproc)

# Unit tests for a specific subsystem
cmake --build build --target osquery_sql_tests -j$(nproc)

# All targets
cmake --build build -j$(nproc)
```

### Build an Extension

```bash
# The example read-only table extension
cmake --build build --target example_extension -j$(nproc)
```

---

## Running Locally

### Interactive Shell (`osqueryi`)

```bash
./build/osquery/osqueryi

# With verbose output
./build/osquery/osqueryi --verbose

# Execute a single query and exit
./build/osquery/osqueryi --json "SELECT * FROM os_version"
```

### Daemon (`osqueryd`)

Create a minimal test configuration:

```bash
mkdir -p /tmp/osquery-dev/logs

cat > /tmp/osquery-dev/osquery.conf << 'EOF'
{
  "options": {
    "logger_path": "/tmp/osquery-dev/logs",
    "disable_logging": false
  },
  "schedule": {
    "uptime": {
      "query": "SELECT * FROM uptime;",
      "interval": 10
    }
  }
}
EOF

./build/osquery/osqueryd \
  --config_path /tmp/osquery-dev/osquery.conf \
  --database_path /tmp/osquery-dev/db \
  --pidfile /tmp/osquery-dev/osqueryd.pid \
  --verbose \
  --disable_watchdog
```

> `--disable_watchdog` is useful during development to avoid the watcher process forking. Remove it in production.

---

## Working with Extensions

Extensions allow you to add custom virtual tables without modifying core osquery. The repository includes four example extensions under `external/examples/`.

### Build the Example Extension

```bash
cmake --build build --target example_extension -j$(nproc)
```

### Run with an Extension Loaded

```bash
# Start osqueryi with an extension autoloaded
./build/osquery/osqueryi \
  --extension ./build/external/examples/read_only_table/example_extension

# Once loaded, query the extension table
osquery> SELECT * FROM example;
+--------------+-----------------+
| example_text | example_integer |
+--------------+-----------------+
| example      | 1               |
+--------------+-----------------+
```

### Create Your Own Extension

Extension development follows this pattern:

```cpp
#include <osquery/sdk/sdk.h>
#include <osquery/sql/dynamic_table_row.h>

using namespace osquery;

// 1. Define your table plugin
class MyTable : public TablePlugin {
 private:
  TableColumns columns() const {
    return {
        std::make_tuple("name",  TEXT_TYPE,    ColumnOptions::DEFAULT),
        std::make_tuple("value", INTEGER_TYPE, ColumnOptions::DEFAULT),
    };
  }

  TableRows generate(QueryContext& request) {
    TableRows results;
    auto r = make_table_row();
    r["name"]  = "my_entry";
    r["value"] = INTEGER(42);
    results.push_back(std::move(r));
    return results;
  }
};

// 2. Register with the extension runtime
REGISTER_EXTERNAL(MyTable, "table", "my_table");

// 3. Standard extension entry point
int main(int argc, char* argv[]) {
  osquery::Initializer runner(argc, argv, ToolType::EXTENSION);
  auto status = startExtension("my_extension", "1.0.0");
  if (!status.ok()) {
    LOG(ERROR) << status.getMessage();
    runner.requestShutdown(status.getCode());
  }
  runner.waitForShutdown();
  return runner.shutdown(0);
}
```

---

## Hot Reload / Watch Mode

osquery does not have a native watch mode for source code, but you can use `entr` or `inotifywait` to auto-rebuild on file changes:

```bash
# Linux: rebuild on any .cpp change in osquery/core
find osquery/core -name '*.cpp' | \
  entr -r cmake --build build --target osqueryi -j$(nproc)
```

For configuration changes, the osqueryd daemon automatically reloads config on the configured `config_refresh` interval (default: 0, meaning manual refresh only). Set a refresh interval:

```json
{
  "options": {
    "config_refresh": 60
  }
}
```

Or send `SIGHUP` to trigger an immediate reload:

```bash
kill -HUP $(cat /tmp/osquery-dev/osqueryd.pid)
```

---

## Debug Configuration

### GDB (Linux)

```bash
# Build with debug symbols
cmake -B build-debug -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --target osqueryi -j$(nproc)

# Launch under GDB
gdb --args ./build-debug/osquery/osqueryi --verbose
(gdb) run
(gdb) bt    # backtrace after a crash
```

### LLDB (macOS)

```bash
lldb ./build-debug/osquery/osqueryi
(lldb) run --verbose
(lldb) bt   # backtrace after a crash
```

### VS Code Launch Configuration (`.vscode/launch.json`)

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug osqueryi",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/osquery/osqueryi",
      "args": ["--verbose"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "lldb",
      "preLaunchTask": "build osqueryi"
    }
  ]
}
```

---

## Useful Development Flags

| Flag | Description |
|---|---|
| `--verbose` | Enable verbose logging |
| `--disable_watchdog` | Run without the watcher supervisor |
| `--disable_events` | Skip eventing subsystem initialization |
| `--disable_logging` | Suppress result logging (useful for testing) |
| `--ephemeral` | Use in-memory database (no disk writes) |
| `--config_path` | Point to a custom config file |
| `--database_path` | Override the RocksDB data directory |

---

## Community

For local development questions, join the **OpenMSP Slack**:

https://www.openmsp.ai/
