# osquery — OpenFrame Edition Documentation

Welcome to the documentation for **osquery with OpenFrame** — the cross-platform OS instrumentation framework extended with AI-driven MSP automation by [Flamingo](https://flamingo.run) and [OpenFrame](https://openframe.ai).

---

## 📚 Table of Contents

- [Getting Started](#-getting-started)
- [Development](#-development)
- [Reference Architecture](#-reference-architecture)
- [Architecture Diagrams](#-architecture-diagrams)
- [Quick Links](#-quick-links)
- [Community](#-community)

---

## 🚀 Getting Started

New to osquery? Start here.

| Guide | Description |
|---|---|
| [Introduction](./getting-started/introduction.md) | What is osquery with OpenFrame? Features and target audience |
| [Prerequisites](./getting-started/prerequisites.md) | System requirements, supported platforms, required software |
| [Quick Start](./getting-started/quick-start.md) | Clone, build, and run osquery in under 10 minutes |
| [First Steps](./getting-started/first-steps.md) | Explore virtual tables, scheduled packs, FIM, extensions, and OpenFrame |

**Recommended reading order:** Introduction → Prerequisites → Quick Start → First Steps

---

## 🛠 Development

Guides for contributors and developers building on or extending osquery.

### Setup

| Guide | Description |
|---|---|
| [Environment Setup](./development/setup/environment.md) | IDE recommendations, clangd, ccache, editor extensions |
| [Local Development](./development/setup/local-development.md) | Build configurations, debug flags, GDB/LLDB, VS Code debugging |

### Architecture

| Guide | Description |
|---|---|
| [Architecture Overview](./development/architecture/README.md) | High-level module breakdown, runtime lifecycle, design decisions |

### Quality

| Guide | Description |
|---|---|
| [Testing Guide](./development/testing/README.md) | GTest/GMock structure, running tests, writing unit and integration tests |
| [Security Best Practices](./development/security/README.md) | Auth patterns, AES-256-GCM, SQL authorizer, TLS, secrets management |

### Contributing

| Guide | Description |
|---|---|
| [Contributing Guidelines](./development/contributing/guidelines.md) | Code style, branch naming, commit format, PR process, virtual table creation |

---

## 📖 Reference Architecture

Deep technical documentation for each core subsystem — generated directly from source code analysis.

| Module | Description |
|---|---|
| [Core Init And Runtime](./reference/architecture/core-init-and-runtime/core-init-and-runtime.md) | Process bootstrap, flags, watcher/worker model, watchdog |
| [SQL Engine And Virtual Tables](./reference/architecture/sql-engine-and-virtual-tables/sql-engine-and-virtual-tables.md) | SQLite engine, authorizer, virtual table binding, constraint pushdown |
| [Configuration And Packs](./reference/architecture/configuration-and-packs/configuration-and-packs.md) | Config loading, packs, schedulers, decorators, change detection |
| [Eventing Framework And Subscriptions](./reference/architecture/eventing-framework-and-subscriptions/eventing-framework-and-subscriptions.md) | Publisher/subscriber system, inotify, BPF, FSEvents, ETW |
| [Extensions And IPC](./reference/architecture/extensions-and-ipc/extensions-and-ipc.md) | Apache Thrift IPC, runtime plugin model, UUID routing |
| [Distributed Querying](./reference/architecture/distributed-querying/distributed-querying.md) | Remote SQL orchestration, TLS transport, fleet denylisting |
| [Remote HTTP Client](./reference/architecture/remote-http-client/remote-http-client.md) | Boost.Asio/Beast HTTPS client, TLS handshake, peer verification |
| [Logging And Query Observability](./reference/architecture/logging-and-query-observability/logging-and-query-observability.md) | Differential result tracking, JSON serialization, pluggable backends |
| [Database And Storage Plugins](./reference/architecture/database-and-storage-plugins/database-and-storage-plugins.md) | RocksDB persistent backend, ephemeral in-memory store, key-value interface |
| [Filesystem And Path Utilities](./reference/architecture/filesystem-and-path-utilities/filesystem-and-path-utilities.md) | Cross-platform file abstraction, glob resolution, permission enforcement |

---

## 🗺 Architecture Diagrams

Visual Mermaid diagrams for each subsystem are available in:

```text
docs/diagrams/architecture/
```

Diagrams cover all major subsystems including the SQL engine, eventing framework, configuration pipeline, distributed querying, OpenFrame auth layer, and more. Open any `.mmd` file in a Mermaid-compatible viewer.

---

## 🔗 Quick Links

| Resource | Link |
|---|---|
| [Project README](../README.md) | Main project overview, quick start, and features |
| [Contributing Guide](../CONTRIBUTING.md) | How to contribute code, docs, and virtual tables |
| [License](../LICENSE.md) | License information |

---

## 💬 Community

> We do **not** use GitHub Issues or GitHub Discussions. All support and collaboration happens on **OpenMSP Slack**.

| Resource | Link |
|---|---|
| OpenMSP Community Slack | [Join here](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) |
| OpenMSP Website | [https://www.openmsp.ai/](https://www.openmsp.ai/) |
| OpenFrame Platform | [https://openframe.ai](https://openframe.ai) |
| Flamingo | [https://flamingo.run](https://flamingo.run) |

---

*Documentation generated by [OpenFrame Doc Orchestrator](https://github.com/flamingo-stack/openframe-oss-tenant)*
