# First Steps

Welcome to osquery! This guide walks you through the first five things to do after a successful installation.

---

## 1. Explore Available Tables

osquery exposes 300+ virtual tables across Linux, macOS, and Windows. Start by discovering what's available in the interactive shell:

```bash
./osquery/osqueryi
```

List all available tables:

```sql
.tables
```

Search for tables matching a keyword:

```sql
.tables process
```

Get the schema of a specific table:

```sql
.schema processes
```

Expected output:

```text
CREATE VIRTUAL TABLE processes USING osquery(
  pid BIGINT,
  name TEXT,
  path TEXT,
  cmdline TEXT,
  state TEXT,
  cwd TEXT,
  root TEXT,
  uid BIGINT,
  ...
);
```

---

## 2. Run Your First Security Queries

Try these essential security-focused queries:

**Find processes running from unusual locations:**

```sql
SELECT pid, name, path
FROM processes
WHERE path NOT LIKE '/usr/%'
  AND path NOT LIKE '/bin/%'
  AND path NOT LIKE '/sbin/%'
  AND on_disk = 0;
```

**List all listening network ports:**

```sql
SELECT pid, port, protocol, address
FROM listening_ports
WHERE address != '127.0.0.1';
```

**Find recently modified files in system directories:**

```sql
SELECT path, mtime, size
FROM file
WHERE directory = '/etc'
  AND mtime > (SELECT unix_time - 3600 FROM time);
```

**Check logged-in users:**

```sql
SELECT username, tty, host, time
FROM logged_in_users;
```

---

## 3. Create Your First Query Pack

Query packs allow you to schedule queries to run at regular intervals. Create a configuration file:

```bash
mkdir -p /etc/osquery
```

```bash
cat > /etc/osquery/osquery.conf << 'EOF'
{
  "options": {
    "config_plugin": "filesystem",
    "logger_plugin": "filesystem",
    "logger_path": "/var/log/osquery",
    "schedule_splay_percent": 10
  },
  "schedule": {
    "running_processes": {
      "query": "SELECT pid, name, path, cmdline FROM processes;",
      "interval": 60,
      "description": "Track all running processes every minute"
    },
    "network_connections": {
      "query": "SELECT pid, local_address, local_port, remote_address, remote_port FROM process_open_sockets WHERE state = 'ESTABLISHED';",
      "interval": 30,
      "description": "Track established network connections every 30 seconds"
    },
    "user_activity": {
      "query": "SELECT username, tty, host, time FROM logged_in_users;",
      "interval": 300,
      "description": "Track user logins every 5 minutes"
    }
  }
}
EOF
```

Start the daemon with this configuration:

```bash
./osquery/osqueryd \
  --config_path=/etc/osquery/osquery.conf \
  --pidfile=/var/run/osqueryd.pid \
  --daemonize=true
```

---

## 4. Understand Event-Based Tables

Some tables capture events in real time rather than querying static state. To enable file integrity monitoring (FIM):

Add file paths to watch in your configuration:

```bash
cat > /etc/osquery/osquery.conf << 'EOF'
{
  "options": {
    "config_plugin": "filesystem",
    "logger_plugin": "filesystem",
    "logger_path": "/var/log/osquery",
    "disable_events": false
  },
  "schedule": {
    "file_events": {
      "query": "SELECT target_path, action, time FROM file_events;",
      "interval": 30
    }
  },
  "file_paths": {
    "etc": [
      "/etc/%%"
    ],
    "home": [
      "/home/%/.ssh/%%"
    ]
  }
}
EOF
```

Query file events interactively:

```sql
SELECT target_path, action, time
FROM file_events
ORDER BY time DESC
LIMIT 20;
```

---

## 5. Configure OpenFrame Integration

To connect this osquery instance with the OpenFrame/Flamingo platform:

### Obtain Your Token File

After authenticating with the OpenFrame platform, you'll receive an encrypted token file. Store it securely:

```bash
sudo mkdir -p /etc/openframe
sudo chmod 700 /etc/openframe
# Copy your encrypted token file here
sudo cp openframe-token.enc /etc/openframe/token.enc
sudo chmod 600 /etc/openframe/token.enc
```

### Configure the Token Path

Set the token file path in your osquery configuration:

```bash
cat > /etc/osquery/osquery.conf << 'EOF'
{
  "options": {
    "config_plugin": "tls",
    "logger_plugin": "tls",
    "tls_hostname": "your-openframe-endpoint.openframe.ai",
    "enroll_secret_path": "/etc/openframe/token.enc"
  }
}
EOF
```

Refer to your OpenFrame environment configuration for the correct hostname and secret key values.

---

## Key Configuration Options Reference

| Option | Description | Example |
|--------|-------------|---------|
| `config_plugin` | Configuration source (`filesystem`, `tls`) | `"filesystem"` |
| `logger_plugin` | Log destination (`filesystem`, `tls`, `syslog`) | `"filesystem"` |
| `logger_path` | Directory for filesystem logs | `"/var/log/osquery"` |
| `schedule_splay_percent` | Random schedule jitter (%) | `10` |
| `disable_events` | Disable event-based tables | `false` |
| `config_refresh` | Config refresh interval (seconds) | `60` |

---

## Where to Get Help

| Resource | Link |
|----------|------|
| **Community Slack** | [OpenMSP](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) |
| **Flamingo Platform** | [flamingo.run](https://flamingo.run) |
| **OpenFrame Platform** | [openframe.ai](https://openframe.ai) |
| **OpenMSP Community** | [openmsp.ai](https://www.openmsp.ai/) |

> **Tip:** The OpenMSP Slack community is the primary support channel. GitHub Issues and Discussions are not used for this project.
