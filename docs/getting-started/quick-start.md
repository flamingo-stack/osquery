# Quick Start

Get osquery (with OpenFrame integration) built and running in under 5 minutes.

---

## TL;DR — 4 Steps to Your First Query

```bash
# 1. Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# 2. Configure the build with CMake
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOSQUERY_BUILD_TESTS=OFF

# 3. Build (use -j with your CPU core count)
cmake --build build --target osqueryi -j$(nproc)

# 4. Run your first query
./build/osqueryi "SELECT name, pid, user_time FROM processes LIMIT 5;"
```

> **Tip:** On macOS replace `$(nproc)` with `$(sysctl -n hw.logicalcpu)`.
> On Windows, use the Visual Studio Developer Command Prompt and CMake's `--parallel` flag.

---

## Windows Quick Start

Download the Windows AMD64 CLI installer:

- [openframe-cli_windows_amd64.zip](https://github.com/flamingo-stack/openframe-cli/releases/latest/download/openframe-cli_windows_amd64.zip)

Then run the installer the same way as other platforms after extracting.

---

## Step-by-Step Walkthrough

### Step 1: Clone the Repository

```bash
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
```

### Step 2: Configure the CMake Build

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOSQUERY_BUILD_TESTS=OFF \
  -DOSQUERY_NO_DEBUG_SYMBOLS=ON
```

Key CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug` | Use `Release` for production |
| `OSQUERY_BUILD_TESTS` | `ON` | Set `OFF` to skip test compilation |
| `OSQUERY_NO_DEBUG_SYMBOLS` | `OFF` | Set `ON` to reduce binary size |
| `OSQUERY_BUILD_EXTENSIONS` | `ON` | Build the extensions framework |

### Step 3: Build osqueryi (Interactive Shell)

```bash
cmake --build build --target osqueryi -j$(nproc)
```

Or build the full daemon:

```bash
cmake --build build --target osqueryd -j$(nproc)
```

### Step 4: Run Your First Query

```bash
./build/osqueryi
```

You should see the interactive prompt:

```text
Using a virtual database. Need help, type '.help'
osquery>
```

---

## "Hello World" — Your First Queries

Once inside `osqueryi`, try these queries to verify everything is working:

```sql
-- System info
SELECT hostname, cpu_type, cpu_brand, physical_memory FROM system_info;

-- Running processes
SELECT name, pid, state FROM processes LIMIT 10;

-- Network interfaces
SELECT interface, address, mask FROM interface_addresses;

-- Listening ports
SELECT port, protocol, pid FROM listening_ports WHERE protocol = 6 LIMIT 5;
```

### Expected Output

```text
osquery> SELECT hostname, cpu_type FROM system_info;
+--------------------+---------+
| hostname           | cpu_type|
+--------------------+---------+
| my-workstation     | x86_64  |
+--------------------+---------+
```

---

## Running as a Daemon (osqueryd)

Create a minimal configuration file:

```json
{
  "options": {
    "logger_path": "/var/log/osquery",
    "database_path": "/var/osquery/osquery.db"
  },
  "schedule": {
    "system_info": {
      "query": "SELECT * FROM system_info;",
      "interval": 3600
    },
    "listening_ports": {
      "query": "SELECT * FROM listening_ports;",
      "interval": 60
    }
  }
}
```

Run the daemon:

```bash
sudo ./build/osqueryd --config_path /etc/osquery/osquery.conf --verbose
```

---

## Enable OpenFrame Mode

If you have an OpenFrame token, enable the integration at startup:

```bash
./build/osqueryd \
  --openframe_mode=true \
  --openframe_token_path=/etc/osquery/openframe_token \
  --config_path=/etc/osquery/osquery.conf
```

The token refresher will start automatically in the background, keeping your authentication credentials up to date.

---

## Verify the Build Succeeded

```bash
# Check the binary exists and is executable
ls -la build/osqueryi build/osqueryd

# Confirm osquery version
./build/osqueryi --version

# Run a quick self-check
./build/osqueryi "SELECT * FROM osquery_info;"
```

---

## Next Steps

After your first successful run:

- Review the [Prerequisites](prerequisites.md) if you encounter build issues
- Work through [First Steps](first-steps.md) to explore key features and configuration
- Join the [OpenMSP Slack community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) for help
