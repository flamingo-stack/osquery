# Local Development Guide

This guide covers cloning the repository, building locally, running osquery in development mode, and configuring a debugger.

---

## Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

Initialize submodules if any are present:

```bash
git submodule update --init --recursive
```

---

## Directory Structure

```text
.
├── osquery/           # Core osquery C++ source
├── openframe/         # OpenFrame authentication extensions
├── plugins/           # Config, logger, database, distributed plugins
├── libraries/         # Vendored third-party libraries
├── tools/             # Code generation scripts, CI tooling
├── external/          # Extension SDK examples
├── tests/             # Integration test suite
├── CMakeLists.txt     # Root CMake configuration
└── package.json       # Documentation pipeline tooling (not a build dep)
```

---

## Build for Local Development

### 1. Create a Build Directory

Always build out-of-source:

```bash
mkdir build
cd build
```

### 2. Configure CMake (Debug Mode)

```bash
cmake .. \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

> **Tip:** `CMAKE_EXPORT_COMPILE_COMMANDS=ON` generates `compile_commands.json` for IDE code intelligence.

### 3. Build All Targets

```bash
cmake --build . --parallel $(nproc)
```

### 4. Build Specific Targets

Build only the interactive shell:

```bash
cmake --build . --target osqueryi --parallel $(nproc)
```

Build only the daemon:

```bash
cmake --build . --target osqueryd --parallel $(nproc)
```

Build the test suite:

```bash
cmake --build . --target osquery_tests --parallel $(nproc)
```

---

## Running Locally

### Interactive Shell (osqueryi)

```bash
./osquery/osqueryi
```

With verbose output:

```bash
./osquery/osqueryi --verbose
```

Disable logging for cleaner development output:

```bash
./osquery/osqueryi --disable_logging=true
```

### Daemon (osqueryd)

Create a minimal development config:

```bash
cat > /tmp/dev_osquery.conf << 'EOF'
{
  "options": {
    "config_plugin": "filesystem",
    "logger_plugin": "filesystem",
    "logger_path": "/tmp/osquery_dev_logs",
    "disable_logging": false,
    "schedule_splay_percent": 0
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_type FROM system_info;",
      "interval": 10
    }
  }
}
EOF
```

Run the daemon in the foreground (useful during development):

```bash
./osquery/osqueryd \
  --config_path=/tmp/dev_osquery.conf \
  --ephemeral=true \
  --disable_database=true \
  --verbose
```

> **Note:** `--ephemeral=true --disable_database=true` uses the in-memory database, avoiding RocksDB file locking issues during rapid iteration.

---

## Incremental Builds

After modifying a source file, CMake only recompiles the changed translation units:

```bash
cd build
cmake --build . --parallel $(nproc)
```

After adding a new source file, re-run CMake first:

```bash
cmake ..
cmake --build . --parallel $(nproc)
```

---

## Debug Configuration

### Using LLDB (macOS / Linux)

```bash
lldb ./osquery/osqueryi
(lldb) run --verbose
```

Set a breakpoint:

```bash
(lldb) breakpoint set --name OpenframeEncryptionService::decrypt
(lldb) run
```

### Using GDB (Linux)

```bash
gdb ./osquery/osqueryi
(gdb) break OpenframeTokenExtractor::extractToken
(gdb) run --verbose
```

### VS Code Debug Configuration

Create `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug osqueryi",
      "type": "lldb",
      "request": "launch",
      "program": "${workspaceFolder}/build/osquery/osqueryi",
      "args": ["--verbose", "--disable_database=true"],
      "cwd": "${workspaceFolder}",
      "preLaunchTask": "cmake-build"
    },
    {
      "name": "Debug osqueryd",
      "type": "lldb",
      "request": "launch",
      "program": "${workspaceFolder}/build/osquery/osqueryd",
      "args": [
        "--config_path=/tmp/dev_osquery.conf",
        "--verbose",
        "--ephemeral=true"
      ],
      "cwd": "${workspaceFolder}"
    }
  ]
}
```

Create `.vscode/tasks.json` for the pre-launch build task:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "cmake-build",
      "type": "shell",
      "command": "cmake --build build --parallel $(nproc)",
      "group": {
        "kind": "build",
        "isDefault": true
      }
    }
  ]
}
```

---

## Running Unit Tests

```bash
cd build

# Run all unit tests
ctest --parallel $(nproc) --output-on-failure

# Run a specific test binary
./osquery/osquery_tests --gtest_filter=ConfigTests.*

# Run with verbose output
./osquery/osquery_tests --gtest_filter=* --gtest_verbose=all
```

---

## Watching for Changes

osquery doesn't have a built-in watch mode. Use `entr` or a similar tool to trigger rebuilds on file changes:

```bash
# Linux/macOS - rebuild on any .cpp or .h change under osquery/ or openframe/
find osquery/ openframe/ -name '*.cpp' -o -name '*.h' | \
  entr -c cmake --build build --parallel $(nproc) --target osqueryi
```

Install `entr` if needed:

```bash
# Ubuntu/Debian
sudo apt-get install entr

# macOS
brew install entr
```

---

## Cleaning the Build

Remove build artifacts without reconfiguring:

```bash
cmake --build build --target clean
```

Full clean (delete build directory):

```bash
rm -rf build/
```

---

## Adding a New OpenFrame Source File

When adding a new C++ source file to the `openframe/` directory:

1. Create the `.h` and `.cpp` files.
2. Add the source to the relevant `CMakeLists.txt`:

```cmake
target_sources(openframe_lib PRIVATE
  openframe/openframe_new_component.cpp
)
```

3. Re-run CMake and rebuild:

```bash
cd build
cmake ..
cmake --build . --parallel $(nproc)
```

---

## Useful CMake Flags for Development

| Flag | Description |
|------|-------------|
| `-DCMAKE_BUILD_TYPE=Debug` | Include debug symbols |
| `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` | Generate `compile_commands.json` |
| `-DOSQUERY_DISABLE_DATABASE=ON` | Skip RocksDB build (faster for table dev) |
| `-DOSQUERY_NO_DEBUG_SYMBOLS=ON` | Reduce binary size |
| `-DCMAKE_C_COMPILER_LAUNCHER=ccache` | Enable ccache for C compilation |
| `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache` | Enable ccache for C++ compilation |
