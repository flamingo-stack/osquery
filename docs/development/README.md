# Development Documentation

Welcome to the osquery with OpenFrame development documentation. This section covers everything you need to build, run, test, and contribute to the project.

---

## Overview

osquery is a C++ cross-platform OS instrumentation framework. The codebase is organized around a modular, plugin-driven architecture where every major subsystem is independently testable and extensible.

The **OpenFrame** layer adds authentication and encryption components that integrate with the [Flamingo/OpenFrame MSP platform](https://openframe.ai).

---

## Documentation Index

| Guide | Description |
|---|---|
| [Environment Setup](setup/environment.md) | IDE recommendations, development tools, editor extensions |
| [Local Development](setup/local-development.md) | Clone, build, run locally, debug configuration |
| [Architecture Overview](architecture/README.md) | High-level architecture diagrams and core component descriptions |
| [Security Best Practices](security/README.md) | Auth patterns, encryption, secrets management |
| [Testing Guide](testing/README.md) | Test structure, running tests, writing new tests |
| [Contributing Guidelines](contributing/guidelines.md) | Code style, branch naming, PR process, commit format |

---

## Technology Stack

| Layer | Technology |
|---|---|
| **Language** | C++17 |
| **Build System** | CMake 3.21+ with Ninja |
| **SQL Engine** | SQLite (embedded, in-memory) |
| **IPC** | Apache Thrift (UNIX sockets / named pipes) |
| **Encryption** | OpenSSL (AES-256-GCM via OpenFrame layer) |
| **Networking** | Boost.Asio + Boost.Beast |
| **Event Systems** | inotify (Linux), FSEvents (macOS), ETW (Windows), BPF (Linux) |
| **Database** | RocksDB (persistent), ephemeral in-memory store |
| **Code Generation** | Python scripts for table schema, API, and amalgamation |
| **Testing** | Google Test + Google Mock |

---

## Repository Structure

```text
osquery/
├── osquery/           # Core C++ source — SQL, eventing, config, logging
│   ├── core/          # Initialization, flags, watcher/worker model
│   ├── sql/           # SQLite engine, virtual tables, authorizer
│   ├── config/        # Configuration loading, packs, parsers
│   ├── events/        # Eventing framework (publishers/subscribers)
│   ├── database/      # Storage backend abstraction (RocksDB, ephemeral)
│   ├── distributed/   # Distributed query orchestration
│   ├── extensions/    # Thrift-based extension/IPC framework
│   ├── remote/        # HTTP client, TLS transport
│   ├── logger/        # Logging plugins and observability
│   └── tables/        # Virtual table implementations
├── openframe/         # OpenFrame authentication and encryption layer
├── plugins/           # Logger, config, database, and distributed plugins
├── libraries/         # CMake-managed third-party dependencies
├── tools/             # Codegen scripts, CI tools, formatting
├── tests/             # Integration test suite
└── external/          # Extension examples
```

---

## Quick Commands

```bash
# Configure build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Build everything
cmake --build build --parallel $(nproc)

# Run interactive shell
./build/osquery/osqueryi

# Run tests
cmake --build build --target osquery_tests
cd build && ctest --output-on-failure
```

---

## Getting Help

All development discussions happen in the **OpenMSP Slack community**:

- [Join OpenMSP Slack](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- [https://www.openmsp.ai/](https://www.openmsp.ai/)

> We do not use GitHub Issues or GitHub Discussions. All support and collaboration is on Slack.
