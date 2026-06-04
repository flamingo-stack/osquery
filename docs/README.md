# osquery — OpenFrame Edition: Documentation

Welcome to the documentation for **osquery with the OpenFrame integration** — the SQL-powered OS instrumentation framework for the [Flamingo](https://flamingo.run) MSP platform.

---

## 📚 Table of Contents

- [Getting Started](#-getting-started)
- [Development](#-development)
- [Reference Architecture](#-reference-architecture)
- [Architecture Diagrams](#-architecture-diagrams)
- [Quick Links](#-quick-links)

---

## 🚀 Getting Started

New to osquery? Start here for a guided introduction, prerequisites, and your first running query.

| Guide | Description |
|-------|-------------|
| [Introduction](./getting-started/introduction.md) | What osquery is, key features, and the OpenFrame integration overview |
| [Prerequisites](./getting-started/prerequisites.md) | Hardware requirements, supported OS, required tools, and environment setup |
| [Quick Start](./getting-started/quick-start.md) | Build and run osquery in under 5 minutes |
| [First Steps](./getting-started/first-steps.md) | Explore key features, scheduled queries, events, and SQL patterns after your first build |

---

## 🛠️ Development

Guides for contributors and platform engineers working on the codebase.

| Guide | Description |
|-------|-------------|
| [Development Overview](./development/README.md) | Technology stack, project layout, quick developer commands, and community links |
| [Environment Setup](./development/setup/environment.md) | IDE recommendations (VS Code, CLion, Xcode, Visual Studio), toolchain setup, ccache, and sanitizers |
| [Local Development](./development/setup/local-development.md) | Clone, configure, build targets, run, watch-mode iteration, and debug configuration |
| [Architecture Overview](./development/architecture/README.md) | System architecture diagrams, boot flow, SQL execution data flow, and key design decisions |
| [Security Guidelines](./development/security/README.md) | Auth patterns, token storage, TLS configuration, input validation, and least-privilege setup |
| [Testing Guide](./development/testing/README.md) | Test structure, running tests, writing unit and integration tests, benchmarks, and CI matrix |
| [Contributing Guidelines](./development/contributing/guidelines.md) | Code style, branching, commit messages, pull request process, and review checklist |

---

## 📖 Reference Architecture

Deep-dive technical reference documentation for every major module, generated from source code analysis.

### Core Runtime

| Module | Description |
|--------|-------------|
| [Core Init And Runtime](./reference/architecture/core-init-and-runtime/core-init-and-runtime.md) | Lifecycle orchestration — flag parsing, plugin activation, watcher/worker model, graceful shutdown |
| [Config And Packs](./reference/architecture/config-and-packs/config-and-packs.md) | Configuration loading, query scheduling, pack management, and config refresh lifecycle |
| [Config Plugins](./reference/architecture/config-plugins/config-plugins.md) | Configuration retrieval and parsing plugin implementations |

### SQL and Query Engine

| Module | Description |
|--------|-------------|
| [SQL Core And Virtual Tables](./reference/architecture/sql-core-and-virtual-tables/sql-core-and-virtual-tables.md) | Embedded SQLite engine, virtual table integration, constraint pushdown, result diffing |
| [Query Execution And Logging](./reference/architecture/query-execution-and-logging/query-execution-and-logging.md) | Differential logging model, query state persistence, epoch and counter management |

### Storage and Events

| Module | Description |
|--------|-------------|
| [Database Backends](./reference/architecture/database-backends/database-backends.md) | RocksDB persistent storage, ephemeral in-memory backend, domain keyspaces, migrations |
| [Events Core](./reference/architecture/events-core/events-core.md) | Publish/subscribe event framework, event publishers, subscriber plugins, backing store |

### Distributed and Networking

| Module | Description |
|--------|-------------|
| [Distributed Querying](./reference/architecture/distributed-querying/distributed-querying.md) | Remote query orchestration, TLS distributed plugin, denylisting, concurrency control |
| [Remote HTTP Client](./reference/architecture/remote-http-client/remote-http-client.md) | Boost.Asio + OpenSSL HTTP/HTTPS transport, timeout handling, proxy support |

### Extensions and Plugins

| Module | Description |
|--------|-------------|
| [Extensions Framework](./reference/architecture/extensions-framework/extensions-framework.md) | Apache Thrift IPC, runtime plugin injection, extension manager, SDK |
| [Plugin Interfaces And Logging](./reference/architecture/plugin-interfaces-and-logging/plugin-interfaces-and-logging.md) | LoggerPlugin interface, status log routing, snapshot and diff result forwarding |

### Utilities

| Module | Description |
|--------|-------------|
| [Filesystem And Fileops](./reference/architecture/filesystem-and-fileops/filesystem-and-fileops.md) | Cross-platform file abstraction, stat normalization, globbing, socket lifecycle |
| [Hashing](./reference/architecture/hashing/hashing.md) | Streaming hash computation, multi-algorithm support (MD5, SHA1, SHA256) |
| [Process And Profiler](./reference/architecture/process-and-profiler/process-and-profiler.md) | Worker and extension spawning, POSIX signal handling, code profiler |
| [System Utilities](./reference/architecture/system-utilities/system-utilities.md) | `EnumClassHash`, time conversion utilities |

---

## 🗺️ Architecture Diagrams

Visual architecture documentation is available as Mermaid diagram files in the diagrams directory:

```text
docs/diagrams/architecture/
```

Diagrams cover all major modules including:

- System overview and end-to-end data flow (`README.mmd`)
- Core Init And Runtime boot flow (`core-init-and-runtime.mmd`)
- SQL Core And Virtual Tables execution (`sql-core-and-virtual-tables.mmd`)
- Query Execution And Logging pipeline (`query-execution-and-logging.mmd`)
- Events Core publish/subscribe model (`events-core.mmd`)
- Distributed Querying orchestration (`distributed-querying.mmd`)
- Extensions Framework IPC (`extensions-framework.mmd`)
- Config And Packs lifecycle (`config-and-packs.mmd`)
- Database Backends storage layer (`database-backends.mmd`)
- Remote HTTP Client transport (`remote-http-client.mmd`)
- Hashing module (`hashing.mmd`)
- Filesystem And Fileops (`filesystem-and-fileops.mmd`)
- Process And Profiler (`process-and-profiler.mmd`)
- Plugin Interfaces And Logging (`plugin-interfaces-and-logging.mmd`)
- Config Plugins (`config-plugins.mmd`)
- System Utilities (`system-utilities.mmd`)

---

## 🔗 Quick Links

| Resource | Link |
|----------|------|
| [Project README](../README.md) | Main project overview, quick start, and feature summary |
| [Contributing Guide](../CONTRIBUTING.md) | How to contribute — code style, branches, commits, PR process |
| [OpenMSP Slack](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) | Community discussion and support (no GitHub Issues) |
| [openmsp.ai](https://www.openmsp.ai/) | OpenMSP community website |
| [flamingo.run](https://flamingo.run) | Flamingo MSP platform |
| [openframe.ai](https://openframe.ai) | OpenFrame unified MSP platform |

---

*Documentation generated by [OpenFrame Doc Orchestrator](https://github.com/flamingo-stack/openframe-oss-tenant)*
