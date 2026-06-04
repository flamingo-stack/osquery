# Development Documentation

Welcome to the osquery (OpenFrame-enhanced) development documentation. This section covers everything you need to understand, build, modify, and contribute to this project.

---

## Overview

This osquery distribution extends the upstream open-source osquery framework with OpenFrame-specific authentication, token management, and AES-256-GCM encryption services. The codebase is written in **C++17** and built with **CMake**, targeting Linux, macOS, and Windows.

---

## Documentation Index

### Setup

| Document | Description |
|----------|-------------|
| [Environment Setup](setup/environment.md) | IDE configuration, development tools, editor extensions |
| [Local Development](setup/local-development.md) | Cloning, building, running locally, debug configuration |

### Architecture

| Document | Description |
|----------|-------------|
| [Architecture Overview](architecture/README.md) | High-level system design, component diagrams, data flow |

### Security

| Document | Description |
|----------|-------------|
| [Security Best Practices](security/README.md) | Auth patterns, encryption, secrets management, vulnerability mitigations |

### Testing

| Document | Description |
|----------|-------------|
| [Testing Overview](testing/README.md) | Test structure, running tests, writing new tests, coverage |

### Contributing

| Document | Description |
|----------|-------------|
| [Contributing Guidelines](contributing/guidelines.md) | Code style, branch naming, PR process, commit format |

---

## Project Structure

```text
osquery/
├── osquery/           # Core osquery source (C++17)
│   ├── core/          # Runtime lifecycle, flags, watchdog
│   ├── sql/           # SQL engine and virtual table layer
│   ├── config/        # Configuration and query packs
│   ├── database/      # RocksDB and ephemeral backends
│   ├── events/        # Event publisher/subscriber system
│   ├── extensions/    # Thrift-based extension framework
│   ├── distributed/   # Distributed query engine
│   ├── logger/        # Logging and query metadata
│   └── remote/        # HTTP client for TLS endpoints
├── openframe/         # OpenFrame-specific extensions (C++)
│   ├── openframe_authorization_manager.*
│   ├── openframe_encryption_service.*
│   ├── openframe_token_extractor.*
│   └── openframe_token_refresher.*
├── plugins/           # Config, logger, database plugins
├── libraries/         # Vendored third-party libraries (CMake)
├── tools/             # Code generation, CI scripts
├── external/          # Extension SDK examples
└── tests/             # Integration tests
```

---

## Technology Stack

| Layer | Technology |
|-------|-----------|
| **Language** | C++17 |
| **Build System** | CMake ≥ 3.21 |
| **SQL Engine** | SQLite (embedded) |
| **Persistent Database** | RocksDB |
| **IPC / Extensions** | Apache Thrift |
| **Networking** | Boost.Asio + Boost.Beast |
| **Encryption** | OpenSSL (AES-256-GCM) |
| **Logging** | Google glog |
| **Testing** | Google Test (gtest) |
| **Flags** | Google gflags |
| **Documentation Pipeline** | VoltAgent + Anthropic AI |

---

## Quick Navigation

- New to the codebase? Start with [Architecture Overview](architecture/README.md).
- Setting up your machine? See [Environment Setup](setup/environment.md).
- Ready to code? Follow [Local Development](setup/local-development.md).
- Writing a fix or feature? Read [Contributing Guidelines](contributing/guidelines.md).
- Security concern? See [Security Best Practices](security/README.md).

---

## Community

All development discussions happen in the [OpenMSP Slack community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA). This project does not use GitHub Issues or GitHub Discussions.
