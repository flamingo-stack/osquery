# First Steps

Congratulations on getting osquery running! This guide walks you through the first 5 things to do after a successful build to start getting value from the platform.

---

## 1. Explore the Interactive Shell

Launch `osqueryi` and get familiar with the built-in meta-commands:

```bash
./build/osqueryi
```

Inside the shell:

```sql
-- List all available tables
.tables

-- Show the schema for a specific table
.schema processes

-- Show help
.help

-- Enable output timing
.timer on

-- Pretty print results
.mode column
.headers on
```

### Useful Discovery Queries

```sql
-- How many tables are available on this platform?
SELECT count(*) FROM osquery_registry WHERE active = 1 AND registry = 'table';

-- What osquery version am I running?
SELECT version, build_platform, build_distro FROM osquery_info;

-- What flags are configured?
SELECT name, value, type, description FROM osquery_flags LIMIT 20;
```

---

## 2. Query System Information

Get a complete picture of the system you are monitoring:

```sql
-- Hardware and OS overview
SELECT hostname, cpu_brand, cpu_physical_cores,
       physical_memory, hardware_vendor, hardware_model
FROM system_info;

-- OS version
SELECT name, version, major, minor, patch, build FROM os_version;

-- Uptime
SELECT days, hours, minutes FROM uptime;

-- Disk usage
SELECT path, type, blocks_size, blocks_available, blocks
FROM mounts WHERE path = '/';
```

---

## 3. Write Your First Scheduled Configuration

Create a configuration file to schedule continuous monitoring:

```json
{
  "options": {
    "logger_path": "/var/log/osquery",
    "database_path": "/var/osquery/osquery.db",
    "schedule_splay_percent": 10
  },
  "schedule": {
    "system_info": {
      "query": "SELECT hostname, cpu_brand, physical_memory FROM system_info;",
      "interval": 3600,
      "description": "Capture system hardware info hourly"
    },
    "processes": {
      "query": "SELECT name, pid, state, user_time FROM processes;",
      "interval": 60,
      "description": "Track running processes every minute"
    },
    "listening_ports": {
      "query": "SELECT pid, port, protocol, family, address FROM listening_ports;",
      "interval": 60,
      "description": "Monitor open network ports"
    },
    "users": {
      "query": "SELECT uid, gid, username, description, shell FROM users;",
      "interval": 3600,
      "description": "Audit user accounts hourly"
    }
  }
}
```

Run the daemon with this config:

```bash
sudo ./build/osqueryd \
  --config_path /etc/osquery/osquery.conf \
  --logger_path /var/log/osquery \
  --verbose
```

---

## 4. Explore Event-Driven Monitoring

osquery's Events Core provides real-time monitoring of system activity. Enable file integrity monitoring (FIM):

```json
{
  "file_paths": {
    "etc": ["/etc/%%"],
    "bin": ["/usr/bin/%%", "/usr/sbin/%%"]
  },
  "schedule": {
    "file_events": {
      "query": "SELECT target_path, action, sha256, md5, time FROM file_events;",
      "interval": 30
    }
  }
}
```

On Linux, process events are available via BPF or inotify:

```sql
-- See recent process events (requires event publisher running)
SELECT pid, parent, path, cmdline, time FROM process_events LIMIT 10;

-- Socket events
SELECT action, local_address, remote_address, local_port, remote_port
FROM socket_events LIMIT 10;
```

---

## 5. Use Query Packs

Query packs let you bundle related queries together. Download community packs or write your own:

```json
{
  "packs": {
    "incident-response": "/etc/osquery/packs/incident-response.conf",
    "vuln-management": "/etc/osquery/packs/vuln-management.conf"
  }
}
```

Example pack file:

```json
{
  "queries": {
    "shell_history": {
      "query": "SELECT uid, time, command, history_file FROM shell_history;",
      "interval": 3600,
      "platform": "posix",
      "description": "Collect shell command history"
    },
    "sudoers": {
      "query": "SELECT header, rule_details FROM sudoers;",
      "interval": 3600,
      "description": "Audit sudoers configuration"
    },
    "ssh_keys": {
      "query": "SELECT uid, path, key_type, comment FROM user_ssh_keys;",
      "interval": 86400,
      "description": "Enumerate SSH keys"
    }
  }
}
```

---

## Key SQL Patterns to Know

### Differential Queries (Change Detection)

osquery logs the _difference_ between query runs by default. This means you only see rows that were added or removed since the last execution — ideal for detecting changes.

```sql
-- Find new processes since last check (differential mode is automatic)
SELECT name, pid, path, cmdline FROM processes;
```

### Joining Tables

```sql
-- Map listening ports to process names and users
SELECT p.name AS process, u.username, lp.port, lp.protocol
FROM listening_ports lp
JOIN processes p ON lp.pid = p.pid
JOIN users u ON p.uid = u.uid
WHERE lp.protocol = 6
ORDER BY lp.port;
```

### Using Subqueries

```sql
-- Find processes running as root that are listening on the network
SELECT name, pid, path
FROM processes
WHERE pid IN (
  SELECT pid FROM listening_ports WHERE address != '127.0.0.1'
) AND uid = 0;
```

---

## Where to Get Help

| Resource | Link |
|----------|------|
| OpenMSP Slack Community | [Join here](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) |
| Community Site | [openmsp.ai](https://www.openmsp.ai/) |
| Flamingo Platform | [flamingo.run](https://flamingo.run) |
| OpenFrame Platform | [openframe.ai](https://openframe.ai) |

---

## Quick Reference — Shell Commands

```text
.tables               List all virtual tables
.schema <table>       Show table column definitions
.mode column          Columnar output format
.headers on           Show column headers
.timer on             Show query execution time
.quit                 Exit the shell
```
