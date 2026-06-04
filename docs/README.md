# osquery (OpenFrame-Enhanced) — Documentation

Welcome to the documentation for the OpenFrame-enhanced distribution of **osquery**, a cross-platform OS instrumentation framework that exposes system state as relational data using SQL. This distribution is part of the [Flamingo](https://flamingo.run) / [OpenFrame](https://openframe.ai) platform for intelligent MSP automation.

---

## 📚 Table of Contents

- [Getting Started](#-getting-started)
- [Development](#-development)
- [Reference Architecture](#-reference-architecture)
- [Architecture Diagrams](#-architecture-diagrams)
- [Quick Links](#-quick-links)

---

## 🚀 Getting Started

New to osquery? Start here.

| Document | Description |
|----------|-------------|
| [Introduction](./getting-started/introduction.md) | What is osquery, key features, OpenFrame extensions, and architecture overview |
| [Prerequisites](./getting-started/prerequisites.md) | System requirements, required build tools, and environment verification |
| [Quick Start](./getting-started/quick-start.md) | Clone, build, and run osquery in minutes |
| [First Steps](./getting-started/first-steps.md) | Explore tables, run security queries, create query packs, and configure OpenFrame integration |

---

## 🛠 Development

Guides for contributors and developers working on the codebase.

| Document | Description |
|----------|-------------|
| [Development Overview](./development/README.md) | Project structure, technology stack, and navigation index |
| [Environment Setup](./development/setup/environment.md) | IDE configuration (CLion, VS Code), compiler setup, ccache, clang-format |
| [Local Development](./development/setup/local-development.md) | Build modes, running locally, incremental builds, debugging with LLDB/GDB |
| [Architecture Overview](./development/architecture/README.md) | High-level system design, component diagrams, data flow, OpenFrame auth pipeline |
| [Security Best Practices](./development/security/README.md) | Auth patterns, AES-256-GCM encryption, secrets management, TLS configuration |

---

## 📖 Reference Architecture

Detailed technical documentation for each core module, auto-generated from source code analysis.

### Core Runtime

| Document | Description |
|----------|-------------|
| [Core Runtime And Lifecycle](./reference/architecture/core-runtime-and-lifecycle/core-runtime-and-lifecycle.md) | Bootstrapping, flag system, process modes, watchdog model, shutdown coordination |

### Query Engine

| Document | Description |
|----------|-------------|
| [SQL Engine And Virtual Tables](./reference/architecture/sql-engine-and-virtual-tables/sql-engine-and-virtual-tables.md) | Embedded SQLite, virtual table integration, constraint pushdown, query planning, security authorizer |

### Configuration

| Document | Description |
|----------|-------------|
| [Configuration And Packs](./reference/architecture/configuration-and-packs/configuration-and-packs.md) | Query packs, scheduling, discovery logic, denylisting, hash-based change detection |
| [Config Plugins](./reference/architecture/config-plugins/config-plugins.md) | FilesystemConfigPlugin, OptionsConfigParserPlugin, ViewsConfigParserPlugin, DecoratorsConfigParserPlugin |

### Storage

| Document | Description |
|----------|-------------|
| [Database Backend](./reference/architecture/database-backend/database-backend.md) | RocksDB persistent backend, ephemeral in-memory backend, domain-based key–value storage, schema migration |

### Observability

| Document | Description |
|----------|-------------|
| [Logging And Query Metadata](./reference/architecture/logging-and-query-metadata/logging-and-query-metadata.md) | QueryLogItem, DiffResults, differential vs snapshot logging, JSON serialization |
| [Events Core And Subscriptions](./reference/architecture/events-core-and-subscriptions/events-core-and-subscriptions.md) | Publisher/subscriber event framework, event persistence, schedule-aware expiration |
| [Osquery Shell Devtooling](./reference/architecture/osquery-shell-devtooling/osquery-shell-devtooling.md) | `osqueryi` interactive shell, meta commands, pretty-print formatting, CPU profiling |

### Distribution & Extensions

| Document | Description |
|----------|-------------|
| [Distributed Querying](./reference/architecture/distributed-querying/distributed-querying.md) | Pull-based distributed queries, discovery gating, denylisting, result batching |
| [Distributed TLS Plugin](./reference/architecture/distributed-tls-plugin/distributed-tls-plugin.md) | Secure HTTPS transport for distributed queries via `TLSDistributedPlugin` |
| [Extensions Framework](./reference/architecture/extensions-framework/extensions-framework.md) | Thrift IPC, dynamic plugin injection, UUID-based routing, health monitoring |

### Networking & Filesystem

| Document | Description |
|----------|-------------|
| [Remote HTTP Client](./reference/architecture/remote-http-client/remote-http-client.md) | Boost.Asio + Boost.Beast HTTP client, TLS validation, timeout handling, proxy support |
| [Filesystem And Fileops Core](./reference/architecture/filesystem-and-fileops-core/filesystem-and-fileops-core.md) | Cross-platform file abstraction, secure permission validation, globbing, portable stat |

---

## 🗺 Architecture Diagrams

Mermaid-format architecture diagrams are available under `docs/diagrams/architecture/`. Key diagrams include:

| Diagram | Description |
|---------|-------------|
| `docs/diagrams/architecture/core-runtime-and-lifecycle.mmd` | Core runtime lifecycle flow |
| `docs/diagrams/architecture/sql-engine-and-virtual-tables.mmd` | SQL engine and virtual table layer |
| `docs/diagrams/architecture/extensions-framework.mmd` | Extensions IPC and lifecycle |
| `docs/diagrams/architecture/events-core-and-subscriptions.mmd` | Event publisher/subscriber pipeline |
| `docs/diagrams/architecture/distributed-querying.mmd` | Distributed query execution flow |
| `docs/diagrams/architecture/distributed-tls-plugin.mmd` | TLS-secured distributed transport |
| `docs/diagrams/architecture/database-backend.mmd` | Database backend architecture |
| `docs/diagrams/architecture/configuration-and-packs.mmd` | Configuration and scheduling flow |
| `docs/diagrams/architecture/config-plugins.mmd` | Config plugin chain |
| `docs/diagrams/architecture/remote-http-client.mmd` | HTTP client TLS flow |
| `docs/diagrams/architecture/filesystem-and-fileops-core.mmd` | Filesystem abstraction layer |
| `docs/diagrams/architecture/logging-and-query-metadata.mmd` | Logging pipeline |
| `docs/diagrams/architecture/osquery-shell-devtooling.mmd` | Interactive shell architecture |

---

## 🔗 Quick Links

| Resource | Link |
|----------|------|
| [Project README](../README.md) | Main project README with Quick Start |
| [Contributing Guide](../CONTRIBUTING.md) | How to contribute, code style, PR process |
| [OpenMSP Community Slack](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) | Community support (primary channel) |
| [OpenMSP Platform](https://www.openmsp.ai/) | OpenMSP community platform |
| [Flamingo](https://flamingo.run) | Flamingo AI-powered MSP platform |
| [OpenFrame](https://openframe.ai) | OpenFrame unified MSP tooling platform |

---

*Documentation generated by [OpenFrame Doc Orchestrator](https://github.com/flamingo-stack/openframe-oss-tenant)*
