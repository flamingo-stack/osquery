# Quick Start

Get osquery built and running in minutes with this step-by-step guide.

---

## TL;DR — 5-Minute Setup

```bash
# 1. Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# 2. Create the build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build osquery
cmake --build . --parallel $(nproc)

# 5. Run the interactive shell
./osquery/osqueryi
```

---

## Step-by-Step Instructions

### 1. Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

### 2. Configure the Build

```bash
mkdir build
cd build
cmake ..
```

Common CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `-DCMAKE_BUILD_TYPE=Release` | `RelWithDebInfo` | Build type (Release/Debug/RelWithDebInfo) |
| `-DCMAKE_C_COMPILER=clang` | System default | C compiler |
| `-DCMAKE_CXX_COMPILER=clang++` | System default | C++ compiler |
| `-DOSQUERY_DISABLE_DATABASE=ON` | OFF | Disable RocksDB (use ephemeral backend) |

Example with explicit Clang:

```bash
cmake .. \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Release
```

### 3. Build

```bash
# Use all available CPU cores for parallel compilation
cmake --build . --parallel $(nproc)
```

> **Tip:** On a 6-core system, this typically takes 20–40 minutes on first build. Subsequent incremental builds are much faster.

### 4. Verify the Build

```bash
# Check the binaries were produced
ls -la osquery/osqueryi osquery/osqueryd
```

Expected output:

```text
-rwxr-xr-x ... osquery/osqueryi
-rwxr-xr-x ... osquery/osqueryd
```

---

## Your First Query

Launch the interactive shell:

```bash
./osquery/osqueryi
```

Try a simple query:

```sql
SELECT name, version FROM os_version;
```

Expected output (example on Ubuntu):

```text
+----------+---------+
| name     | version |
+----------+---------+
| Ubuntu   | 22.04   |
+----------+---------+
```

Try querying running processes:

```sql
SELECT pid, name, path FROM processes LIMIT 5;
```

Try querying network interfaces:

```sql
SELECT interface, address, mask FROM interface_addresses;
```

List all available tables:

```sql
.tables
```

Exit the shell:

```sql
.exit
```

---

## Running osqueryd (Daemon Mode)

The daemon mode runs scheduled queries continuously using a configuration file.

Create a minimal configuration file:

```bash
cat > /tmp/osquery.conf << 'EOF'
{
  "options": {
    "config_plugin": "filesystem",
    "logger_plugin": "filesystem",
    "logger_path": "/tmp/osquery_logs",
    "disable_logging": false
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_type, physical_memory FROM system_info;",
      "interval": 60
    }
  }
}
EOF
```

Run the daemon:

```bash
./osquery/osqueryd --config_path=/tmp/osquery.conf --pidfile=/tmp/osquery.pid
```

Check logs:

```bash
ls /tmp/osquery_logs/
cat /tmp/osquery_logs/osqueryd.results.log
```

---

## Windows Installation

On Windows (AMD64), you can download the pre-built CLI:

1. Download: [openframe-cli_windows_amd64.zip](https://github.com/flamingo-stack/openframe-cli/releases/latest/download/openframe-cli_windows_amd64.zip)
2. Extract the archive
3. Run the installer following the same steps as other operating systems

---

## Expected Build Output

A successful build produces:

```text
[100%] Built target osqueryi
[100%] Built target osqueryd
```

The following binaries are available:

| Binary | Description |
|--------|-------------|
| `osquery/osqueryi` | Interactive SQL shell |
| `osquery/osqueryd` | Continuous monitoring daemon |

---

## Next Steps

After completing this quick start:

- Read [First Steps](first-steps.md) to explore key features and configuration options.
- Review [Prerequisites](prerequisites.md) if you encountered build issues.
