# Development Documentation

Welcome to the osquery development documentation. This section covers everything needed to set up a development environment, understand the architecture, write tests, follow security best practices, and contribute to the codebase.

---

## Contents

| Guide | Description |
|---|---|
| [Environment Setup](setup/environment.md) | IDE configuration, editor plugins, and development tooling |
| [Local Development](setup/local-development.md) | Cloning, building, running, and debugging locally |
| [Architecture Overview](architecture/README.md) | High-level design, core components, and data flow |
| [Security Guidelines](security/README.md) | Authentication, secrets management, and secure coding |
| [Testing Overview](testing/README.md) | Test structure, running tests, and coverage |
| [Contributing Guidelines](contributing/guidelines.md) | Code style, branch strategy, PR process, and review checklist |

---

## Quick Navigation

### I want to…

**Set up my development environment:**
→ Start with [Local Development](setup/local-development.md)

**Understand how osquery works internally:**
→ Read the [Architecture Overview](architecture/README.md)

**Write a new virtual table:**
→ See [Local Development](setup/local-development.md) for the build setup, then read the [Architecture Overview](architecture/README.md) for the virtual table framework

**Run the test suite:**
→ Go to [Testing Overview](testing/README.md)

**Submit a pull request:**
→ Read [Contributing Guidelines](contributing/guidelines.md) first

**Understand security considerations:**
→ See [Security Guidelines](security/README.md)

---

## Repository Layout

```text
osquery/
├── external/examples/         # Extension SDK examples
│   ├── config_plugin/         # Example config plugin extension
│   ├── read_only_table/       # Example read-only virtual table
│   ├── string_batch/          # Example batch string table
│   └── writable_table/        # Example writable virtual table
├── libraries/cmake/source/    # Bundled third-party libraries
├── openframe/                 # Flamingo OpenFrame integration layer
├── osquery/                   # Core osquery source
│   ├── carver/                # File carving subsystem
│   ├── config/                # Configuration loading and management
│   ├── core/                  # Init, flags, shutdown, watcher
│   ├── database/              # RocksDB / ephemeral persistence
│   ├── devtools/              # Interactive shell (osqueryi)
│   ├── dispatcher/            # Thread pool and scheduler
│   ├── distributed/           # Distributed query execution
│   ├── events/                # Eventing core and platform publishers
│   ├── extensions/            # Thrift IPC and extension manager
│   ├── filesystem/            # Cross-platform file abstraction
│   ├── hashing/               # MD5/SHA1/SHA256 utilities
│   ├── logger/                # Logger plugin infrastructure
│   ├── main/                  # Binary entry points
│   ├── numeric_monitoring/    # Performance metric plugins
│   ├── process/               # Process abstraction layer
│   ├── profiler/              # Code profiler utilities
│   ├── registry/              # Plugin registry factory
│   ├── remote/                # HTTP/HTTPS client and TLS transport
│   ├── sql/                   # SQLite integration and virtual tables
│   ├── system/                # Network hostname, user/group services
│   ├── tables/                # All virtual table implementations
│   ├── utils/                 # Cross-platform utilities
│   └── worker/                # Worker IPC and logging bridge
├── plugins/                   # Pluggable config, logger, and DB backends
├── tests/                     # Integration test specs
└── tools/                     # Build tools, CI scripts, code generation
    ├── ci/                    # CI helper scripts
    ├── cmake/                 # CMake helper scripts
    ├── codegen/               # Table and API code generators
    └── tests/                 # Python integration test runner
```

---

## Development Philosophy

osquery is designed around several key principles:

1. **Portability first** — Every feature targets Linux, macOS, and Windows.
2. **Plugin-based extensibility** — Tables, loggers, config sources, and databases are all plugins.
3. **Security by default** — SQLite opcodes are allowlisted; file permissions are enforced.
4. **Process isolation** — The watcher/worker model limits blast radius from buggy queries.
5. **SQL as the interface** — All system data is accessible through standard SQL.

---

## Community

Questions, ideas, and discussions are handled in the **OpenMSP Slack community**:

https://www.openmsp.ai/
