# Quick Start

This guide gets osquery built from source and running in interactive mode in as few steps as possible.

---

## TL;DR

```bash
# 1. Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# 2. Configure the build (Linux / macOS)
cmake -B build -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo

# 3. Build osqueryi (interactive shell)
cmake --build build --target osqueryi -j$(nproc)

# 4. Run the interactive shell
./build/osquery/osqueryi
```

On **Windows** (PowerShell):

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config RelWithDebInfo --target osqueryi
.\build\osquery\RelWithDebInfo\osqueryi.exe
```

---

## Step-by-Step

### 1. Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

> The repository bundles most of its dependencies under `libraries/cmake/source/` — no separate dependency installation step is required for most platforms.

---

### 2. Configure the Build

osquery uses **CMake** as its build system. Choose a build type:

| Build Type | Description |
|---|---|
| `Debug` | Full debug symbols, no optimizations |
| `RelWithDebInfo` | Optimized + debug symbols (recommended for dev) |
| `Release` | Fully optimized, minimal symbols |

```bash
cmake -B build -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

> Ninja is the recommended generator. You can also use `Unix Makefiles` if Ninja is unavailable.

---

### 3. Build Targets

Build only what you need:

```bash
# Build the interactive shell only (fastest)
cmake --build build --target osqueryi -j$(nproc)

# Build the daemon
cmake --build build --target osqueryd -j$(nproc)

# Build everything
cmake --build build -j$(nproc)
```

> The first build will compile all bundled libraries and may take **20–60 minutes** depending on hardware.

---

### 4. Run the Interactive Shell

```bash
./build/osquery/osqueryi
```

You will be greeted with the osquery SQL shell:

```text
Using a virtual database. Need help, type '.help'
osquery>
```

---

## Hello World: Your First Queries

Once in the `osqueryi` shell, try these queries:

```sql
-- What OS is this system running?
SELECT * FROM os_version;

-- What processes are currently running?
SELECT pid, name, path FROM processes LIMIT 10;

-- What network ports are listening?
SELECT pid, port, protocol FROM listening_ports;

-- What users exist on this system?
SELECT uid, username, shell FROM users;

-- What's the current uptime?
SELECT * FROM uptime;
```

Example output for `os_version`:

```text
+----------+--------+-------+-------+-------+-------+----------+
| name     | major  | minor | patch | build | arch  | platform |
+----------+--------+-------+-------+-------+-------+----------+
| Ubuntu   | 22     | 04    | 0     |       | x86_64| ubuntu   |
+----------+--------+-------+-------+-------+-------+----------+
```

---

## Useful Shell Commands

| Command | Description |
|---|---|
| `.tables` | List all available virtual tables |
| `.schema <table>` | Show columns and types for a table |
| `.mode line` | Switch to line-per-column output |
| `.mode pretty` | Switch to tabular output (default) |
| `.help` | Show all shell commands |
| `.exit` | Exit the shell |

```sql
-- Discover tables related to processes
.tables process

-- Inspect the processes table schema
.schema processes
```

---

## Run the Daemon

To run osquery as a scheduled query daemon:

```bash
# Create a minimal configuration file
cat > /tmp/osquery.conf << 'EOF'
{
  "options": {
    "logger_path": "/tmp/osquery_logs",
    "disable_logging": false
  },
  "schedule": {
    "os_version": {
      "query": "SELECT * FROM os_version;",
      "interval": 60
    },
    "listening_ports": {
      "query": "SELECT pid, port, protocol FROM listening_ports;",
      "interval": 30
    }
  }
}
EOF

# Run the daemon
./build/osquery/osqueryd \
  --flagfile /tmp/osquery.conf \
  --verbose
```

> The daemon runs scheduled queries on their configured intervals and logs results to the `logger_path` directory.

---

## Next Steps

After completing this quick start:

- Follow the [First Steps Guide](first-steps.md) to explore key features in depth
- Review [Prerequisites](prerequisites.md) if you encounter build issues
