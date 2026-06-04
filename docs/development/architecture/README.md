# Architecture Overview

This document describes the high-level architecture of osquery with the OpenFrame integration. For deep-dive reference documentation, see the individual module docs in `docs/reference/architecture/`.

---

## System Architecture

osquery is a layered, plugin-driven C++ application. At its core is an embedded SQLite engine that exposes OS state through virtual tables. An event framework captures real-time activity, a distributed query system handles remote work, and the OpenFrame integration provides authenticated connectivity to the Flamingo MSP platform.

### High-Level Architecture Diagram

```mermaid
flowchart TD
    CLI["osqueryi / osqueryd"] --> Core["Core Init And Runtime"]

    Core --> Config["Config And Packs"]
    Core --> Registry["Plugin Registry"]
    Core --> Database["Database Backends (RocksDB)"]
    Core --> SQL["SQL Core And Virtual Tables"]
    Core --> Events["Events Core (pub/sub)"]
    Core --> Distributed["Distributed Querying"]
    Core --> Logger["Plugin Interfaces And Logging"]
    Core --> Extensions["Extensions Framework (Thrift)"]
    Core --> HTTP["Remote HTTP Client (Boost.Beast)"]
    Core --> OpenFrame["OpenFrame Integration"]

    Config --> SQL
    SQL --> QueryExec["Query Execution And Logging"]
    QueryExec --> Logger
    Events --> SQL
    Distributed --> SQL
    Distributed --> HTTP
    OpenFrame --> AuthMgr["Authorization Manager"]
    OpenFrame --> Refresher["Token Refresher Thread"]
```

---

## Core Components

| Module | Location | Responsibility |
|--------|----------|---------------|
| **Core Init And Runtime** | `osquery/core/` | Bootstrapping, flag parsing, lifecycle |
| **SQL Core And Virtual Tables** | `osquery/sql/` | SQLite engine, virtual table integration, constraint pushdown |
| **Events Core** | `osquery/events/` | Publish/subscribe framework for real-time OS events |
| **Config And Packs** | `osquery/config/` | Configuration loading, query scheduling, pack management |
| **Database Backends** | `osquery/database/` | RocksDB (persistent) and ephemeral key-value storage |
| **Distributed Querying** | `osquery/distributed/` | Pull-execute-push model for remote query orchestration |
| **Extensions Framework** | `osquery/extensions/` | Apache Thrift-based runtime plugin injection |
| **Remote HTTP Client** | `osquery/remote/` | Boost.Asio + Boost.Beast HTTP/HTTPS with TLS |
| **Hashing** | `osquery/hashing/` | MD5, SHA1, SHA256 for integrity verification |
| **Filesystem And Fileops** | `osquery/filesystem/` | Cross-platform file access primitives |
| **Plugin Interfaces And Logging** | `osquery/core/plugins/` | Logger plugin interface, status log routing |
| **OpenFrame Integration** | `openframe/` | Token lifecycle, encryption, and auth management |

---

## Runtime Boot Flow

The `Initializer` class in Core Init And Runtime orchestrates the entire startup sequence:

```mermaid
flowchart TD
    Main["Main Entry"] --> Initializer["Initializer"]
    Initializer --> Flags["Parse Flags"]
    Flags --> Mode["Determine Tool Type (daemon/shell/extension)"]
    Mode --> RegistryInit["Registry And Plugin Init"]
    RegistryInit --> DBInit["Database Init And Upgrade"]
    DBInit --> ExtMgr["Extension Manager Start"]
    ExtMgr --> ConfigLoad["Config Load"]
    ConfigLoad --> LoggerInit["Logger Plugin Init"]
    LoggerInit --> DistInit["Distributed Plugin Init"]
    DistInit --> Events["Attach Events"]
    Events --> Dispatcher["Dispatcher Services (scheduler loop)"]
    Dispatcher --> ShutdownReq{"Shutdown Requested?"}
    ShutdownReq -->|"Yes"| GracefulShutdown["Graceful Shutdown"]
    GracefulShutdown --> End["Process Exit"]
```

---

## SQL Execution Data Flow

Every SQL query follows this path from user input to result rows:

```mermaid
sequenceDiagram
    participant User
    participant SQLInternal
    participant SQLiteDBManager
    participant VirtualTable
    participant TablePlugin

    User->>SQLInternal: Execute SQL
    SQLInternal->>SQLiteDBManager: Get DB Instance
    SQLiteDBManager->>SQLiteDBManager: Apply authorizer
    SQLiteDBManager->>VirtualTable: xBestIndex / xFilter
    VirtualTable->>TablePlugin: generate(QueryContext)
    TablePlugin-->>VirtualTable: TableRows
    VirtualTable-->>SQLInternal: Result Set
    SQLInternal-->>User: QueryDataTyped
```

The key security feature here is the SQLite **authorizer** — it enforces an allowlist of permitted SQL actions and PRAGMA statements. Any non-allowlisted operation is denied at prepare time.

---

## Event Framework Architecture

The Events Core implements a publish/subscribe pattern with persistent backing storage:

```mermaid
flowchart TD
    Config["Configuration"] --> Factory["EventFactory (singleton)"]

    subgraph publishers["Event Publishers (platform-specific)"]
        PubLinux["BPF / inotify (Linux)"]
        PubMac["EndpointSecurity (macOS)"]
        PubWin["ETW (Windows)"]
    end

    subgraph subscribers["Event Subscribers"]
        SubFile["File Events Subscriber"]
        SubProc["Process Events Subscriber"]
        SubSock["Socket Events Subscriber"]
    end

    PubLinux -->|"fires events"| SubFile
    PubLinux -->|"fires events"| SubProc
    PubMac -->|"fires events"| SubFile
    PubWin -->|"fires events"| SubProc

    SubFile -->|"addBatch(rows)"| Store["Time-indexed Backing Store (RocksDB)"]
    SubProc -->|"addBatch(rows)"| Store
    SubSock -->|"addBatch(rows)"| Store

    Store -->|"genTable(time window)"| SQL["Virtual Table Query"]

    Factory --> PubLinux
    Factory --> PubMac
    Factory --> PubWin
```

---

## OpenFrame Integration Architecture

The OpenFrame integration is an authentication layer that runs alongside the core runtime when `openframe_mode` is enabled:

```mermaid
flowchart TD
    Flag["--openframe_mode=true"] --> Encryption["OpenframeEncryptionService"]
    Encryption --> Extractor["OpenframeTokenExtractor"]
    Extractor --> AuthMgr["OpenframeAuthorizationManager (singleton)"]
    Extractor --> Refresher["OpenframeTokenRefresher (background thread)"]
    AuthMgr --> APICall["Authenticated OpenFrame API Calls"]
    Refresher -->|"periodic re-read"| Extractor
```

The `OpenframeAuthorizationManager` is a singleton that uses the **provider pattern** — only `OpenframeAuthorizationManagerProvider` can construct or destroy it, separating token lifecycle from token access.

---

## Plugin Registry

The plugin registry (`osquery/registry/`) is the dependency injection backbone of the entire system. Every major subsystem — loggers, config sources, databases, SQL engines, distributed plugins — is registered and resolved through it.

```mermaid
flowchart LR
    RegistryFactory["RegistryFactory"] --> LoggerReg["logger registry"]
    RegistryFactory --> ConfigReg["config registry"]
    RegistryFactory --> DatabaseReg["database registry"]
    RegistryFactory --> TableReg["table registry"]
    RegistryFactory --> DistributedReg["distributed registry"]

    LoggerReg --> TLSLogger["TLS Logger Plugin"]
    LoggerReg --> FSLogger["Filesystem Logger Plugin"]
    ConfigReg --> TLSConfig["TLS Config Plugin"]
    DatabaseReg --> RocksDB["RocksDB Plugin"]
    TableReg --> ProcessesTable["processes virtual table"]
```

---

## Extensions Framework

Extensions allow external processes to add virtual tables, loggers, and config sources at runtime:

```mermaid
sequenceDiagram
    participant Ext as Extension Process
    participant EM as Extension Manager
    participant Reg as RegistryFactory
    participant SQL as SQL Engine

    Ext->>EM: registerExtension(info, registry)
    EM->>Reg: addBroadcast(uuid, routes)
    EM-->>Ext: return UUID
    Ext->>Ext: Start Thrift server
    SQL->>EM: callExtension(uuid, table, query)
    EM->>Ext: call() via Thrift
    Ext-->>SQL: TableRows
```

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| **Embedded SQLite** | No external database dependency; strict authorizer provides security |
| **Plugin Registry** | All major components are swappable; enables testing with stubs |
| **RocksDB for events** | High write throughput for real-time event ingestion |
| **Apache Thrift for extensions** | Language-agnostic IPC; extensions can be written in any language |
| **Boost.Beast for HTTP** | Production-grade async networking; TLS enforced by default |
| **Provider pattern for OpenFrame auth** | Strict access control to token lifecycle prevents accidental mutation |

---

## Reference Documentation

For detailed module-level documentation, see:

- [./reference/architecture/core-init-and-runtime/core-init-and-runtime.md](./reference/architecture/core-init-and-runtime/core-init-and-runtime.md)
- [./reference/architecture/sql-core-and-virtual-tables/sql-core-and-virtual-tables.md](./reference/architecture/sql-core-and-virtual-tables/sql-core-and-virtual-tables.md)
- [./reference/architecture/events-core/events-core.md](./reference/architecture/events-core/events-core.md)
- [./reference/architecture/config-and-packs/config-and-packs.md](./reference/architecture/config-and-packs/config-and-packs.md)
- [./reference/architecture/database-backends/database-backends.md](./reference/architecture/database-backends/database-backends.md)
- [./reference/architecture/distributed-querying/distributed-querying.md](./reference/architecture/distributed-querying/distributed-querying.md)
- [./reference/architecture/extensions-framework/extensions-framework.md](./reference/architecture/extensions-framework/extensions-framework.md)
- [./reference/architecture/remote-http-client/remote-http-client.md](./reference/architecture/remote-http-client/remote-http-client.md)
- [./reference/architecture/hashing/hashing.md](./reference/architecture/hashing/hashing.md)
