# Local Development Guide

This guide explains how to clone, configure, build, run, and debug osquery locally for development.

---

## Clone and Initial Setup

```bash
# Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# Initialize and update all submodules
git submodule update --init --recursive
```

> **Note:** The `libraries/` directory contains vendored third-party libraries managed via CMake's FetchContent and submodules. The initial clone with submodules can take several minutes depending on your connection speed.

---

## Configure the Build

### Development (Debug) Build

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -G Ninja
```

### Release Build

```bash
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DOSQUERY_BUILD_TESTS=OFF \
  -DOSQUERY_NO_DEBUG_SYMBOLS=ON \
  -G Ninja
```

### CMake Configuration Reference

| Option | Description | Development Value |
|--------|-------------|------------------|
| `CMAKE_BUILD_TYPE` | Optimization level | `Debug` |
| `OSQUERY_BUILD_TESTS` | Compile test targets | `ON` |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | Generate IntelliSense DB | `ON` |
| `OSQUERY_NO_DEBUG_SYMBOLS` | Strip debug symbols | `OFF` |
| `OSQUERY_BUILD_EXTENSIONS` | Build extension support | `ON` |
| `OSQUERY_ENABLE_ASAN` | Address Sanitizer | `OFF` (enable as needed) |

---

## Build Targets

```bash
# Build the interactive shell only
cmake --build build --target osqueryi -j$(nproc)

# Build the daemon only
cmake --build build --target osqueryd -j$(nproc)

# Build everything (all targets including tests)
cmake --build build -j$(nproc)

# Build a specific module's tests
cmake --build build --target osquery_sql_tests -j$(nproc)
```

> On macOS, replace `$(nproc)` with `$(sysctl -n hw.logicalcpu)`.

---

## Running Locally

### Interactive Shell (osqueryi)

The fastest way to test SQL queries and table implementations:

```bash
./build/osqueryi
```

Or run a single query directly:

```bash
./build/osqueryi "SELECT name, pid FROM processes LIMIT 5;"
```

Run with verbose logging:

```bash
./build/osqueryi --verbose --minloglevel=0
```

### Daemon Mode (osqueryd)

Create a minimal dev config:

```json
{
  "options": {
    "logger_path": "/tmp/osquery-dev/logs",
    "database_path": "/tmp/osquery-dev/osquery.db",
    "disable_events": false
  },
  "schedule": {
    "test_query": {
      "query": "SELECT * FROM system_info;",
      "interval": 30
    }
  }
}
```

Run the daemon:

```bash
mkdir -p /tmp/osquery-dev/logs
./build/osqueryd \
  --config_path /tmp/osquery-dev/osquery.conf \
  --verbose \
  --minloglevel=0 \
  --disable_watchdog
```

---

## Watch Mode — Iterative Development

For rapid iteration on virtual table implementations, use a shell loop to auto-rebuild on file changes:

```bash
# Linux with inotifywait
while inotifywait -e modify osquery/sql/*.cpp osquery/sql/*.h; do
  cmake --build build --target osqueryi -j$(nproc) && \
  echo "Build succeeded" || echo "Build FAILED"
done
```

Or use `entr` for a cleaner experience:

```bash
# Install entr first: apt-get install entr / brew install entr
find osquery/sql -name "*.cpp" -o -name "*.h" | \
  entr -r cmake --build build --target osqueryi -j$(nproc)
```

---

## Debug Configuration

### VS Code Debug Launch Config

Create `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug osqueryi",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/osqueryi",
      "args": ["--verbose", "--minloglevel=0"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing for gdb",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    },
    {
      "name": "Debug osqueryd",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/osqueryd",
      "args": [
        "--config_path=/tmp/osquery-dev/osquery.conf",
        "--verbose",
        "--disable_watchdog"
      ],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "MIMode": "gdb"
    }
  ]
}
```

### Adding Log Verbosity

```bash
# Maximum verbosity — shows all internal log messages
./build/osqueryi \
  --verbose \
  --minloglevel=0 \
  --stderrthreshold=0 \
  --logger_plugin=stdout
```

### Useful Debug Flags

| Flag | Description |
|------|-------------|
| `--verbose` | Enable verbose output |
| `--minloglevel=0` | Show all log levels (INFO, WARNING, ERROR) |
| `--stderrthreshold=0` | Mirror logs to stderr |
| `--disable_watchdog` | Disable the watcher process (easier debugging) |
| `--disable_events` | Disable event publishers (faster startup) |
| `--database_path=:memory:` | Use ephemeral in-memory database |

---

## Testing During Development

```bash
# Run all unit tests
cd build && ctest --output-on-failure

# Run a specific test binary
./build/osquery_sql_tests

# Run with verbose test output
./build/osquery_sql_tests --gtest_verbose=1

# Run a specific test case
./build/osquery_core_tests --gtest_filter="FlagsTests.*"
```

---

## Working with the OpenFrame Integration

The OpenFrame integration lives in `openframe/`. To develop and test it:

```bash
# Build with OpenFrame support explicitly enabled
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_OPENFRAME=ON

# Run with OpenFrame mode enabled (requires a token file)
echo "your-bearer-token-here" > /tmp/openframe_token
./build/osqueryd \
  --openframe_mode=true \
  --openframe_token_path=/tmp/openframe_token \
  --verbose
```

The `OpenframeTokenRefresher` will start a background thread that re-reads the token file at its configured interval.

---

## Cleaning Up

```bash
# Clean build artifacts
cmake --build build --target clean

# Full rebuild from scratch
rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```
