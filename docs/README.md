# osquery Documentation

Welcome to the documentation for **osquery** — the SQL-powered system telemetry engine maintained by [Flamingo](https://flamingo.run) as part of the [OpenFrame platform](https://openframe.ai).

> osquery exposes operating system state as a relational database, enabling operators to query processes, users, files, network sockets, hardware, and real-time events using standard SQL.

---

## 📚 Table of Contents

- [Getting Started](#getting-started)
- [Development](#development)
- [Reference Architecture](#reference-architecture)
- [Architecture Diagrams](#architecture-diagrams)
- [Quick Links](#quick-links)

---

## 🚀 Getting Started

New to osquery? Start here.

| Guide | Description |
|---|---|
| [Introduction](./getting-started/introduction.md) | What osquery is, key features, architecture overview, and target audience |
| [Prerequisites](./getting-started/prerequisites.md) | Hardware requirements, supported OS, required software, and verification steps |
| [Quick Start](./getting-started/quick-start.md) | Clone, build, and run osquery in minutes |
| [First Steps](./getting-started/first-steps.md) | Explore tables, write security queries, configure scheduled queries, and enable FIM |

---

## 🛠️ Development

Guides for contributors and developers working on the osquery codebase.

### Setup

| Guide | Description |
|---|---|
| [Local Development](./development/setup/local-development.md) | Cloning, building, running, debugging, and working with the Extension SDK |
| [Environment Setup](./development/setup/environment.md) | IDE configuration, compiler setup, clang-format, and development tooling |

### Architecture

| Guide | Description |
|---|---|
| [Architecture Overview](./development/architecture/README.md) | High-level design, core components, process model, SQL lifecycle, eventing, and key design decisions |

### Testing

| Guide | Description |
|---|---|
| [Testing Overview](./development/testing/README.md) | Test structure, building and running tests, writing unit/integration tests, benchmarks, and CI |

### Security

| Guide | Description |
|---|---|
| [Security Guidelines](./development/security/README.md) | Authentication, encryption, SQL authorizer, secrets management, and common vulnerability mitigations |

### Contributing

| Guide | Description |
|---|---|
| [Contributing Guidelines](./development/contributing/guidelines.md) | Code style, branch naming, commit format, PR process, and review checklist |

---

## 📖 Reference Architecture

Detailed technical documentation for each osquery subsystem, generated from source code analysis.

| Module | Description |
|---|---|
| [SQL Core & Virtual Tables](./reference/architecture/sql-core-and-virtual-tables/sql-core-and-virtual-tables.md) | SQLite embedding, virtual table framework, diff engine, and query lifecycle |
| [Eventing Core](./reference/architecture/eventing-core/eventing-core.md) | Publisher–subscriber OS event framework, EventFactory, subscriptions, and PathSet |
| [Core Init, Shutdown & Watcher](./reference/architecture/core-init-shutdown-and-watcher/core-init-shutdown-and-watcher.md) | Process lifecycle, watcher/worker model, watchdog resource enforcement |
| [Core Config & Flags](./reference/architecture/core-config-and-flags/core-config-and-flags.md) | Configuration loading, schedules, packs, flag management, and dynamic reconfiguration |
| [Database](./reference/architecture/database/database.md) | Pluggable key-value persistence, RocksDB backend, ephemeral fallback, and schema migrations |
| [Logging](./reference/architecture/logging/logging.md) | Pluggable logger plugins, StatusLogLine, FilesystemLoggerPlugin, and logging modes |
| [Distributed Querying](./reference/architecture/distributed-querying/distributed-querying.md) | Remote SQL orchestration over TLS, denylisting, performance tracking, and result buffering |
| [Extensions & IPC](./reference/architecture/extensions-and-ipc/extensions-and-ipc.md) | Thrift-based RPC, extension lifecycle, registry broadcasting, and health monitoring |
| [Filesystem & Fileops](./reference/architecture/filesystem-and-fileops/filesystem-and-fileops.md) | Cross-platform file abstraction, permission enforcement, globbing, and read limits |
| [Hashing](./reference/architecture/hashing/hashing.md) | Streaming SHA-256, SHA-1, MD5; single-pass multi-hash; file and buffer hashing |
| [Remote HTTP](./reference/architecture/remote-http/remote-http.md) | Boost.Beast HTTPS client, TLS handling, proxy support, and timeout management |

---

## 🗺️ Architecture Diagrams

Visual Mermaid diagrams for each subsystem are located in:

```text
docs/diagrams/architecture/
```

Diagrams are available for all major modules including SQL Core, Eventing Core, Distributed Querying, Extensions, Database, Logging, and more.

---

## 🔗 Quick Links

| Resource | Link |
|---|---|
| **Project README** | [../README.md](../README.md) |
| **Contributing Guide** | [../CONTRIBUTING.md](../CONTRIBUTING.md) |
| **Source Repository** | [https://github.com/flamingo-stack/osquery](https://github.com/flamingo-stack/osquery) |
| **OpenMSP Slack** | [https://www.openmsp.ai/](https://www.openmsp.ai/) |
| **Flamingo** | [https://flamingo.run](https://flamingo.run) |
| **OpenFrame Platform** | [https://openframe.ai](https://openframe.ai) |

---

## Community

All collaboration, questions, and discussions happen on the **OpenMSP Slack** community:

- 💬 **Join:** [https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- 🌐 **OpenMSP:** [https://www.openmsp.ai/](https://www.openmsp.ai/)

> We do not use GitHub Issues or GitHub Discussions. All support and development discussions take place on Slack.

---

*Documentation generated by [🦩 Flamingo AI Technical Writer](https://flamingo.run)*
