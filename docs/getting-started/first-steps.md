# First Steps

After installing or building osquery, here are the five most important things to do to get productive quickly.

---

## 1. Explore Available Tables

osquery ships with over 300 virtual tables covering virtually every aspect of the operating system. The first step is to discover what's available.

```sql
-- In osqueryi: list all tables
.tables

-- Search for process-related tables
.tables process

-- Search for network-related tables
.tables network
```

Common table categories:

| Category | Example Tables |
|---|---|
| **Processes** | `processes`, `process_open_files`, `process_open_sockets`, `listening_ports` |
| **Users & Groups** | `users`, `groups`, `logged_in_users`, `last` |
| **Packages** | `deb_packages`, `rpm_packages`, `homebrew_packages`, `chocolatey_packages` |
| **Files & FS** | `file`, `hash`, `extended_attributes`, `mounts` |
| **Network** | `interface_addresses`, `routes`, `arp_cache`, `dns_resolvers` |
| **Events** | `file_events`, `process_events`, `socket_events` |
| **Hardware** | `cpu_info`, `usb_devices`, `pci_devices`, `battery` |
| **Kernel** | `kernel_info`, `kernel_modules`, `kernel_extensions` |
| **Cloud** | `ec2_instance_metadata`, `azure_instance_metadata` |

---

## 2. Inspect Table Schemas

Before writing queries, understand the schema of any table:

```sql
-- View the processes table schema
.schema processes

-- View the file table schema
.schema file

-- View listening_ports schema
.schema listening_ports
```

Example output for `.schema processes`:

```text
CREATE TABLE processes(
  pid BIGINT,
  name TEXT,
  path TEXT,
  cmdline TEXT,
  state TEXT,
  cwd TEXT,
  root TEXT,
  uid BIGINT,
  gid BIGINT,
  euid BIGINT,
  ...
);
```

---

## 3. Write Your First Security Queries

These queries are immediately useful for security investigation:

```sql
-- Find processes running as root
SELECT pid, name, path, cmdline
FROM processes
WHERE uid = 0;

-- Find world-writable SUID binaries
SELECT path, permissions
FROM suid_bin;

-- Find SSH authorized keys for all users
SELECT username, key
FROM users
JOIN authorized_keys USING (uid);

-- Find processes with network connections
SELECT p.pid, p.name, lp.address, lp.port
FROM processes p
JOIN listening_ports lp ON p.pid = lp.pid;

-- Check for known malware file hashes (replace with real hash)
SELECT path, sha256
FROM hash
WHERE path LIKE '/usr/bin/%'
  AND sha256 = 'replace_with_suspicious_hash';
```

---

## 4. Configure Scheduled Queries

To run osquery in daemon mode with scheduled queries, create a configuration file:

```json
{
  "options": {
    "logger_path": "/var/log/osquery",
    "disable_logging": false,
    "schedule_splay_percent": 10
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_brand, physical_memory FROM system_info;",
      "interval": 3600
    },
    "users": {
      "query": "SELECT uid, username, shell FROM users;",
      "interval": 600,
      "description": "Track user accounts"
    },
    "listening_ports": {
      "query": "SELECT pid, port, protocol, family, address FROM listening_ports;",
      "interval": 60,
      "description": "Track listening network ports"
    },
    "processes": {
      "query": "SELECT pid, name, path, cmdline, uid FROM processes;",
      "interval": 30,
      "description": "Track running processes"
    }
  },
  "packs": {
    "incident-response": "/etc/osquery/packs/incident-response.conf"
  }
}
```

Start the daemon pointing at this config:

```bash
sudo osqueryd \
  --config_path /etc/osquery/osquery.conf \
  --logger_path /var/log/osquery \
  --pidfile /var/run/osqueryd.pid \
  --daemonize
```

---

## 5. Enable File Integrity Monitoring (FIM)

osquery's eventing system allows you to monitor file changes in real time.

Add a `file_paths` section to your configuration:

```json
{
  "file_paths": {
    "etc": [
      "/etc/%%"
    ],
    "ssh": [
      "/home/%/.ssh/%%",
      "/root/.ssh/%%"
    ],
    "binaries": [
      "/usr/bin/%%",
      "/usr/local/bin/%%"
    ]
  }
}
```

Then query the event table:

```sql
-- Recent file changes (requires osqueryd with eventing)
SELECT time, action, category, path, md5
FROM file_events
ORDER BY time DESC
LIMIT 20;
```

> FIM requires osqueryd (not osqueryi) to be running with the `file_paths` configuration active.

---

## Where to Get Help

| Resource | Link |
|---|---|
| **OpenMSP Slack** | https://www.openmsp.ai/ |
| **Source Code** | https://github.com/flamingo-stack/osquery |
| **Report Issues / Discussion** | https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA |

---

## Key Configuration Flags Reference

| Flag | Default | Description |
|---|---|---|
| `--config_path` | `/etc/osquery/osquery.conf` | Path to the primary config file |
| `--logger_path` | `/var/log/osquery` | Directory for query result logs |
| `--pidfile` | `/var/run/osqueryd.pid` | PID file location |
| `--verbose` | `false` | Enable verbose status output |
| `--disable_watchdog` | `false` | Disable the watchdog supervisor |
| `--watchdog_memory_limit` | `200` | Max worker memory in MB |
| `--watchdog_utilization_limit` | `10` | Max CPU utilization percent |
| `--disable_events` | `false` | Disable the eventing subsystem |
| `--extensions_autoload` | (empty) | Path to file listing extension paths |
