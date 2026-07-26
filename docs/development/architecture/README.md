# Architecture Overview

osquery is a modular, layered system built around a SQL execution engine that treats the operating system as a relational database. This document provides a high-level architectural overview, describes each core component, and illustrates key data flows.

---

## High-Level Architecture

```mermaid
flowchart TD
    Config["Core Config & Flags"] --> Scheduler["Scheduler"]
    Scheduler --> SQLCore["SQL Core & Virtual Tables"]

    SQLCore --> SQLite["Embedded SQLite"]
    SQLite --> VTables["Virtual Tables"]
    VTables --> TablePlugins["Table Plugins / Extensions"]

    SQLCore --> DiffEngine["Diff Engine"]
    DiffEngine --> LogItem["QueryLogItem"]

    LogItem --> Logging["Logging Module"]
    LogItem --> Distributed["Distributed Querying"]

    Distributed --> RemoteHTTP["Remote HTTPS"]

    SQLCore --> Database["RocksDB Database"]

    Eventing["Eventing Core"] --> Database
    Eventing --> SQLCore

    Extensions["Extensions & IPC"] --> TablePlugins
    Extensions --> Config
    Extensions --> Logging

    Watcher["Init / Shutdown / Watcher"] --> Scheduler
    Watcher --> Extensions
```

---

## Core Components

| Module | Location | Responsibility |
|---|---|---|
| **Core Init/Shutdown/Watcher** | `osquery/core/` | Process lifecycle, multi-process supervision |
| **Core Config & Flags** | `osquery/config/` | Configuration loading, scheduling, flag management |
| **SQL Core & Virtual Tables** | `osquery/sql/` | SQLite embedding, virtual table framework, query execution |
| **Eventing Core** | `osquery/events/` | Publisher–subscriber OS event collection |
| **Database** | `osquery/database/` | RocksDB/ephemeral key-value persistence |
| **Logging** | `osquery/logger/` | Pluggable log emission (filesystem, TLS, Kafka, etc.) |
| **Distributed Querying** | `osquery/distributed/` | Remote SQL orchestration over TLS |
| **Extensions & IPC** | `osquery/extensions/` | Thrift-based cross-process plugin framework |
| **Filesystem & Fileops** | `osquery/filesystem/` | Cross-platform file abstraction |
| **Hashing** | `osquery/hashing/` | MD5/SHA1/SHA256 file and buffer digests |
| **Remote HTTP** | `osquery/remote/` | Boost.Asio/Beast HTTPS client for outbound connections |

---

## Process Model

osquery implements a **watcher/worker** supervision model:

```mermaid
flowchart TD
    Init["Initializer"] --> RoleCheck["Determine Role"]

    RoleCheck -->|"Watcher"| WatcherProc["Watcher Process"]
    RoleCheck -->|"Worker"| WorkerProc["Worker Process"]
    RoleCheck -->|"Shell"| ShellProc["osqueryi (Shell Mode)"]

    WatcherProc -->|"spawn"| WorkerProc
    WatcherProc -->|"monitor"| Extensions["Extension Processes"]

    WorkerProc --> Config["Config System"]
    WorkerProc --> SQL["SQL Engine"]
    WorkerProc --> Events["Eventing Core"]
    WorkerProc --> Logger["Logging"]
```

- The **Watcher** (parent process) monitors the worker and all extension processes, enforcing CPU and memory limits.
- The **Worker** (child process) executes all scheduled queries and runs the event subsystem.
- If the worker exceeds resource limits or crashes, the watcher restarts it and may denylist the offending query.
- **Extensions** are separate processes communicating with the core over Thrift/IPC.

---

## SQL Query Lifecycle

This is the path taken by a single scheduled SQL query from configuration to log output:

```mermaid
sequenceDiagram
    participant Scheduler
    participant SQLCore as SQL Core
    participant SQLite
    participant VTable as Virtual Table
    participant TablePlugin as Table Plugin
    participant DiffEngine as Diff Engine
    participant Logger

    Scheduler->>SQLCore: Execute scheduled query
    SQLCore->>SQLite: sqlite3_exec()
    SQLite->>VTable: xFilter() callback
    VTable->>TablePlugin: generate(context)
    TablePlugin-->>VTable: TableRows
    VTable-->>SQLite: Row data
    SQLite-->>SQLCore: QueryData
    SQLCore->>DiffEngine: Compute diff against prior result
    DiffEngine-->>SQLCore: DiffResults (added/removed rows)
    SQLCore->>Logger: Emit QueryLogItem
```

### Key Security Enforcement Points

- **SQLite Authorizer** — Only allowlisted opcodes are permitted. ATTACH and dangerous PRAGMAs are denied.
- **Required constraints** — Tables with required columns (e.g., `file` requires `path`) are enforced at the virtual table layer.
- **Watchdog limits** — Queries consuming too much CPU or memory cause worker restart and query denylisting.

---

## Eventing System

osquery's event tables capture real-time OS activity via a publisher–subscriber model:

```mermaid
flowchart TD
    OS["Operating System Event"] --> Publisher["EventPublisher"]
    Publisher --> Subscriber["EventSubscriber"]
    Subscriber --> Database[("Event Backing Store")]
    Database --> SQLCore["SQL Engine"]
```

### Platform Publishers

| Platform | Publishers |
|---|---|
| **Linux** | inotify, BPF/eBPF, auditd netlink, udev, syslog |
| **macOS** | FSEvents, IOKit, OpenBSM, EndpointSecurity, SCNetwork |
| **Windows** | ETW (Event Tracing for Windows), WEL, NTFS journal, USN journal |

Event data is persisted in the database and exposed as SQL tables like `file_events`, `process_events`, `socket_events`, and `bpf_process_events`.

---

## Extension & IPC Architecture

osquery is designed to be extended without modifying the core binary:

```mermaid
flowchart LR
    Core["osquery Core"] -->|"starts"| Manager["Extension Manager (Thrift)"]
    ExtProc["Extension Process"] -->|"registerExtension()"| Manager
    Manager -->|"assigns UUID"| ExtProc
    Core -->|"callExtension()"| ExtProc
    ExtProc -->|"Thrift Response"| Core
```

Extensions communicate over:
- **UNIX domain sockets** (Linux/macOS)
- **Named pipes** (Windows)

Extensions can provide:
- Custom virtual tables
- Config plugins (alternative config sources)
- Logger plugins (alternative log destinations)
- Distributed plugins (alternative orchestration backends)

---

## Database Layer

```mermaid
flowchart TD
    Client["Core Modules"] -->|"IDatabaseInterface"| OsqueryDB["OsqueryDatabase"]
    OsqueryDB --> API["Global Database APIs"]
    API --> Registry["Plugin Registry"]
    Registry -->|"Active Plugin"| Plugin["DatabasePlugin"]
    Plugin --> RocksDB["RocksDB (persistent default)"]
    Plugin --> Ephemeral["Ephemeral (in-memory fallback)"]
```

Domains stored in the database:

| Domain | Contents |
|---|---|
| `queries` | Scheduled query result state (for diff computation) |
| `events` | Persisted event rows from subscribers |
| `distributed` | Distributed query request tracking |
| `distributed_running` | Active distributed execution state |
| `configurations` | Cached config data |
| `query_performance` | Execution time and resource metrics |
| `carves` | File carving session state |

---

## Configuration Lifecycle

```mermaid
flowchart TD
    Start["Refresh Triggered"] --> CallPlugin["Call genConfig On Active Plugin"]
    CallPlugin --> CheckStatus{"Success?"}
    CheckStatus -->|"No"| Accelerate["Use Accelerated Refresh Interval"]
    CheckStatus -->|"Yes"| UpdateConfig["Update Internal State"]
    UpdateConfig --> Reconfigure["Reconfigure Registries & Loggers"]
    Reconfigure --> Done["Done"]
    Accelerate --> Done
```

Config sources are pluggable:
- **Filesystem** (`--config_path`) — JSON file(s) on disk
- **TLS** (`tls` config plugin) — Remote server via HTTPS
- **Extension** — Any custom plugin registered via the extension framework

---

## Distributed Querying

```mermaid
sequenceDiagram
    participant Server as Remote Server
    participant Agent as osquery Agent
    participant SQL as SQL Engine

    Agent->>Server: POST read_endpoint (getQueries)
    Server-->>Agent: JSON query list
    Agent->>SQL: Execute each query
    SQL-->>Agent: QueryData results
    Agent->>Server: POST write_endpoint (writeResults)
```

Queries are denylisted (via SHA-256 hash) if they crash the worker or consume excessive resources.

---

## Key Design Decisions

1. **Single embedded SQLite instance** — Managed by `SQLiteDBManager` as a singleton with transient connection support for concurrency.
2. **Plugin registry pattern** — All extensible components (tables, loggers, config sources, databases) are registered as plugins, enabling runtime composition.
3. **Process isolation** — The watcher/worker model isolates query execution failures from the supervision layer.
4. **Allowlist-only SQL** — The SQLite authorizer denies all dangerous opcodes by default, preventing SQL-injection-style attacks on the agent itself.
5. **Differential results** — Scheduled queries only log changes (added/removed rows), dramatically reducing log volume.

---

## Reference Documentation

For deeper dives into each module, see the reference architecture docs:

- [SQL Core & Virtual Tables](../../reference/architecture/sql-core-and-virtual-tables/sql-core-and-virtual-tables.md)
- [Eventing Core](../../reference/architecture/eventing-core/eventing-core.md)
- [Core Init/Shutdown/Watcher](../../reference/architecture/core-init-shutdown-and-watcher/core-init-shutdown-and-watcher.md)
- [Core Config & Flags](../../reference/architecture/core-config-and-flags/core-config-and-flags.md)
- [Database](../../reference/architecture/database/database.md)
- [Logging](../../reference/architecture/logging/logging.md)
- [Distributed Querying](../../reference/architecture/distributed-querying/distributed-querying.md)
- [Extensions & IPC](../../reference/architecture/extensions-and-ipc/extensions-and-ipc.md)
- [Filesystem & Fileops](../../reference/architecture/filesystem-and-fileops/filesystem-and-fileops.md)
- [Hashing](../../reference/architecture/hashing/hashing.md)
- [Remote HTTP](../../reference/architecture/remote-http/remote-http.md)
