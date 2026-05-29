# First Steps

Now that osquery is running, here are the first things to explore and configure.

---

## 1. Explore Available Tables

osquery ships with hundreds of virtual tables. Discover them with the `.tables` command in the interactive shell:

```bash
./build/osquery/osqueryi
```

```sql
-- List all available tables
osquery> .tables

-- Search for tables by name
osquery> .tables process
osquery> .tables network
osquery> .tables user

-- Inspect the schema of any table
osquery> .schema processes
osquery> .schema listening_ports
osquery> .schema users
```

Tables are organized by platform:

| Category | Example Tables |
|---|---|
| **System** | `system_info`, `uptime`, `cpu_info`, `memory_info` |
| **Processes** | `processes`, `process_open_files`, `process_open_sockets` |
| **Networking** | `listening_ports`, `interface_addresses`, `arp_cache`, `routes` |
| **Users & Groups** | `users`, `groups`, `logged_in_users`, `sudoers` |
| **Packages** | `deb_packages`, `rpm_packages`, `homebrew_packages`, `chocolatey_packages` |
| **Events** | `file_events`, `process_events`, `socket_events` |
| **Security** | `authorized_keys`, `certificates`, `secureboot`, `disk_encryption` |

---

## 2. Write Your First Scheduled Query Pack

Create a configuration pack to run queries on a schedule:

```bash
sudo tee /etc/osquery/packs/first-steps.json <<'EOF'
{
  "queries": {
    "users_snapshot": {
      "query": "SELECT uid, username, shell, directory FROM users;",
      "interval": 3600,
      "description": "Snapshot of all local users every hour"
    },
    "listening_ports": {
      "query": "SELECT pid, port, protocol, address FROM listening_ports;",
      "interval": 300,
      "description": "Check open ports every 5 minutes"
    },
    "new_processes": {
      "query": "SELECT pid, name, path, cmdline, start_time FROM processes;",
      "interval": 60,
      "description": "Poll running processes every minute"
    }
  }
}
EOF
```

Reference it in your `osquery.conf`:

```json
{
  "options": {
    "logger_plugin": "filesystem",
    "schedule_splay_percent": 10
  },
  "packs": {
    "first-steps": "/etc/osquery/packs/first-steps.json"
  }
}
```

---

## 3. Enable File Integrity Monitoring

osquery can monitor filesystem changes using platform-native event APIs (inotify on Linux, FSEvents on macOS, ETW on Windows):

```json
{
  "file_paths": {
    "etc": ["/etc/%%"],
    "homes": ["/root/.ssh/%%", "/home/%/.ssh/%%"],
    "sensitive": ["/usr/bin/%%", "/usr/sbin/%%"]
  },
  "schedule": {
    "file_events": {
      "query": "SELECT * FROM file_events;",
      "interval": 30
    }
  }
}
```

Then query the event table:

```sql
osquery> SELECT time, target_path, action, md5 FROM file_events;
```

---

## 4. Connect to OpenFrame (Flamingo Platform)

If you are part of the [OpenFrame](https://openframe.ai) / [Flamingo](https://flamingo.run) platform:

1. Obtain your OpenFrame credentials from your platform administrator
2. Configure the TLS enrollment endpoint in your daemon flags
3. The `OpenframeAuthorizationManager` handles token lifecycle automatically
4. The `OpenframeTokenRefresher` keeps sessions active in the background

The OpenFrame layer uses AES-256-GCM encryption via the `OpenframeEncryptionService` — all credential handling is fully automated once configured.

Refer to your environment configuration for the specific connection details.

---

## 5. Load an Extension

osquery's extension system lets you add custom tables, loggers, and configuration plugins as separate processes:

```bash
# Start osqueryd with extension support enabled
sudo ./build/osquery/osqueryd \
  --config_path=/etc/osquery/osquery.conf \
  --extensions_socket=/var/osquery/osquery.em \
  --extensions_autoload=/etc/osquery/extensions.load

# In a separate terminal, run your extension
./my_extension --socket=/var/osquery/osquery.em
```

Extensions communicate via Apache Thrift over UNIX domain sockets (Linux/macOS) or named pipes (Windows).

---

## Key Configuration Flags

| Flag | Description | Default |
|---|---|---|
| `--config_path` | Path to the JSON configuration file | Platform-specific |
| `--logger_plugin` | Logging backend (`filesystem`, `tls`, `syslog`) | `filesystem` |
| `--schedule_splay_percent` | Random splay to distribute query load | `10` |
| `--extensions_socket` | Path to the Thrift extension socket | Platform-specific |
| `--config_refresh` | How often (seconds) to refresh remote config | `0` (disabled) |
| `--watchdog_level` | Resource limit profile (0=normal, 1=restrictive, -1=disabled) | `0` |

---

## Where to Get Help

| Resource | Link |
|---|---|
| **OpenMSP Community Slack** | [Join here](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) |
| **OpenMSP Website** | [https://www.openmsp.ai/](https://www.openmsp.ai/) |
| **Flamingo Platform** | [https://flamingo.run](https://flamingo.run) |
| **OpenFrame** | [https://openframe.ai](https://openframe.ai) |

> All support and discussions are managed on the **OpenMSP Slack community** — not GitHub Issues or GitHub Discussions.

---

## Watch: osquery in Action

[![osquery Overview](https://img.youtube.com/vi/1UcWGiHbLVo/hqdefault.jpg)](https://www.youtube.com/watch?v=1UcWGiHbLVo)
