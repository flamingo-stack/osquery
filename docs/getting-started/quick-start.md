# Quick Start

Get osquery with OpenFrame running in under 10 minutes.

---

## TL;DR — Fastest Path

```bash
# 1. Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# 2. Configure the build
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -G Ninja

# 3. Build osquery (this takes 5–30 minutes depending on hardware)
cmake --build build --parallel $(nproc)

# 4. Run the interactive shell
./build/osquery/osqueryi
```

> **Hardware note**: Building from source requires at minimum 24 GB RAM and 6 CPU cores. See the [Prerequisites Guide](prerequisites.md) for full system requirements.

---

## Step 1: Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

---

## Step 2: Configure the Build

### Linux / macOS

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -G Ninja
```

### Windows (Visual Studio)

```bash
cmake -S . -B build ^
  -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
  -G "Visual Studio 17 2022"
```

### Common CMake Options

| Option | Description |
|---|---|
| `-DCMAKE_BUILD_TYPE=RelWithDebInfo` | Optimized build with debug symbols (recommended) |
| `-DCMAKE_BUILD_TYPE=Debug` | Full debug build (slower, larger binaries) |
| `-DCMAKE_BUILD_TYPE=Release` | Fully optimized production build |
| `-G Ninja` | Use Ninja for faster parallel builds |
| `-DOSQUERY_BUILD_TESTS=ON` | Include test suite in the build |

---

## Step 3: Build

```bash
# Linux/macOS — use all available CPU cores
cmake --build build --parallel $(nproc)

# Windows — use all available CPU cores
cmake --build build --parallel %NUMBER_OF_PROCESSORS%
```

The first build downloads and compiles all third-party dependencies. Subsequent builds with `ccache` are significantly faster.

---

## Step 4: Run Your First Query

Launch the osquery interactive shell:

```bash
./build/osquery/osqueryi
```

Expected output:

```text
Using a virtual database. Need help, type '.help'
osquery>
```

Now try your first query:

```sql
osquery> SELECT hostname, cpu_brand, physical_memory FROM system_info;
```

Example output:

```text
+------------------+-------------------------------+------------------+
| hostname         | cpu_brand                     | physical_memory  |
+------------------+-------------------------------+------------------+
| my-linux-host    | Intel(R) Core(TM) i9-12900K   | 34207285248      |
+------------------+-------------------------------+------------------+
```

---

## Step 5: Explore More Tables

```sql
-- List all running processes
osquery> SELECT pid, name, cmdline FROM processes LIMIT 10;

-- Check listening network ports
osquery> SELECT pid, port, protocol, address FROM listening_ports LIMIT 10;

-- List installed packages (Linux)
osquery> SELECT name, version, arch FROM deb_packages LIMIT 10;

-- Show users on the system
osquery> SELECT uid, gid, username, shell FROM users;
```

---

## Step 6: Run as a Daemon

To run osquery as a background daemon with scheduled queries:

```bash
# Create a basic configuration
sudo mkdir -p /etc/osquery
sudo tee /etc/osquery/osquery.conf <<'EOF'
{
  "options": {
    "logger_plugin": "filesystem",
    "schedule_splay_percent": 10
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_brand, physical_memory FROM system_info;",
      "interval": 3600
    }
  }
}
EOF

# Run the daemon
sudo ./build/osquery/osqueryd --config_path=/etc/osquery/osquery.conf
```

---

## Expected Results Summary

| Command | What You Should See |
|---|---|
| `osqueryi` | Interactive SQL prompt |
| `SELECT * FROM system_info;` | Host identity, CPU, RAM details |
| `SELECT * FROM processes LIMIT 5;` | Running process list |
| `osqueryd` | Daemon starts, logs to `/var/log/osquery/` |

---

## Next Steps

After completing this quick start:

- Follow the [First Steps Guide](first-steps.md) to explore key features and common workflows
- Review the [Prerequisites Guide](prerequisites.md) for full system and software requirements
- Explore the [Development section](../development/README.md) to understand the architecture and contribute
