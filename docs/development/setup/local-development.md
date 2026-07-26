# Local Development Guide

This guide walks you through cloning, building, running, and debugging osquery with OpenFrame on your local machine.

---

## Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

---

## Build Configurations

### Debug Build (Recommended for Development)

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build build --parallel $(nproc)
```

> Debug builds include full debug symbols and disable optimizations, making debugging straightforward.

### RelWithDebInfo (Recommended for Testing)

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

cmake --build build --parallel $(nproc)
```

### Release Build

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build --parallel $(nproc)
```

### Build with Tests Enabled

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON

cmake --build build --parallel $(nproc)
```

---

## Running osquery Locally

### Interactive Shell (`osqueryi`)

```bash
./build/osquery/osqueryi
```

```sql
osquery> SELECT hostname, cpu_brand FROM system_info;
osquery> .tables
osquery> .schema processes
osquery> .quit
```

### Daemon (`osqueryd`)

```bash
# Create a minimal development config
cat > /tmp/osquery-dev.conf <<'EOF'
{
  "options": {
    "logger_plugin": "filesystem",
    "logger_path": "/tmp/osquery-dev-logs",
    "database_path": "/tmp/osquery-dev.db",
    "schedule_splay_percent": 0
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_brand FROM system_info;",
      "interval": 60
    }
  }
}
EOF

mkdir -p /tmp/osquery-dev-logs

./build/osquery/osqueryd \
  --config_path=/tmp/osquery-dev.conf \
  --verbose \
  --ephemeral
```

---

## Development Flags

These flags are useful during local development:

| Flag | Purpose |
|---|---|
| `--verbose` | Enable verbose logging output |
| `--ephemeral` | Use in-memory database (no disk writes) |
| `--disable_watchdog` | Disable resource watchdog (easier debugging) |
| `--disable_events` | Disable event publishers (faster startup) |
| `--disable_logging` | Suppress all logger output |
| `--allow_unsafe` | Allow loading unsigned extensions |
| `--extensions_timeout=10` | Extension connection timeout in seconds |

Example for a minimal debug session:

```bash
./build/osquery/osqueryi \
  --verbose \
  --disable_events \
  --ephemeral
```

---

## Incremental Builds

After making source changes, only rebuild changed targets:

```bash
# Rebuild only the osqueryi binary
cmake --build build --target osqueryi

# Rebuild only the osqueryd binary
cmake --build build --target osqueryd

# Rebuild a specific test target
cmake --build build --target osquery_core_tests
```

---

## Debugging

### Using GDB (Linux)

```bash
# Build with debug symbols
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel $(nproc)

# Launch with GDB
gdb ./build/osquery/osqueryi

# In GDB:
# (gdb) set args --verbose --ephemeral
# (gdb) break osquery::Initializer::start
# (gdb) run
```

### Using LLDB (macOS)

```bash
lldb ./build/osquery/osqueryi

# In LLDB:
# (lldb) settings set -- target.run-args "--verbose" "--ephemeral"
# (lldb) b osquery::Initializer::start
# (lldb) run
```

### Using VS Code Debugger

Add to `.vscode/launch.json`:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug osqueryi",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/build/osquery/osqueryi",
      "args": ["--verbose", "--ephemeral", "--disable_events"],
      "stopAtEntry": false,
      "cwd": "${workspaceFolder}",
      "environment": [],
      "externalConsole": false,
      "MIMode": "gdb",
      "miDebuggerPath": "/usr/bin/gdb",
      "setupCommands": [
        {
          "description": "Enable pretty-printing",
          "text": "-enable-pretty-printing",
          "ignoreFailures": true
        }
      ]
    }
  ]
}
```

---

## Developing the OpenFrame Layer

The OpenFrame components live in the `openframe/` directory:

```text
openframe/
├── openframe_authorization_manager.h/.cpp   # JWT token lifecycle
├── openframe_authorization_manager_provider.h
├── openframe_encryption_service.h/.cpp      # AES-256-GCM via OpenSSL
├── openframe_token_extractor.h/.cpp         # Token acquisition
└── openframe_token_refresher.h/.cpp         # Background token renewal
```

When modifying OpenFrame components, rebuild only the affected target:

```bash
cmake --build build --target openframe_auth
```

Test the encryption service independently by linking against it in a test binary.

---

## Working with Virtual Tables

To add a new virtual table:

1. Create schema and implementation in `osquery/tables/<category>/`
2. Register the table in the appropriate CMake target
3. Run the code generator to update schema definitions:

```bash
python3 tools/codegen/gentable.py osquery/tables/my_category/my_table.table
```

4. Rebuild and test:

```bash
cmake --build build --target osqueryi
./build/osquery/osqueryi
osquery> .tables my_table
```

---

## Log Output Locations

| Mode | Default Log Path |
|---|---|
| Daemon (Linux) | `/var/log/osquery/` |
| Daemon (macOS) | `/var/log/osquery/` |
| Daemon (Windows) | `C:\ProgramData\osquery\log\` |
| Development override | Set `--logger_path=/tmp/my-logs` |

---

## Clean Rebuild

If you encounter stale build artifacts:

```bash
rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel $(nproc)
```
