# Contributing to osquery (OpenFrame-Enhanced)

Thank you for your interest in contributing to the OpenFrame-enhanced distribution of osquery! This guide covers everything you need to get started: setting up your environment, understanding the codebase, following code style conventions, and submitting changes.

---

## Community First

This project is managed through the **OpenMSP Slack community**. GitHub Issues and GitHub Discussions are **not used**.

- **Slack**: [Join OpenMSP](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- **Platform**: [https://www.openmsp.ai/](https://www.openmsp.ai/)

Before opening a pull request, please discuss your planned change in the Slack community. This ensures alignment on scope, approach, and priority.

---

## Development Environment Setup

### System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **RAM** | 24 GB | 32 GB |
| **CPU Cores** | 6 cores | 12 cores |
| **Disk Space** | 50 GB | 100 GB |

### Required Tools

| Tool | Version | Purpose |
|------|---------|---------|
| **CMake** | ≥ 3.21 | Build system generator |
| **Clang** | ≥ 13 | C++ compiler (C++17 required) |
| **Python 3** | ≥ 3.6 | Code generation scripts |
| **OpenSSL** | ≥ 1.1.1 | AES-256-GCM encryption |
| **Git** | ≥ 2.x | Source control |

### Install Dependencies

**Linux (Ubuntu/Debian):**

```bash
sudo apt-get update && sudo apt-get install -y \
  build-essential \
  cmake \
  clang-13 \
  clang++-13 \
  lld-13 \
  python3 \
  python3-pip \
  git \
  libssl-dev \
  ninja-build \
  ccache \
  clang-format-13
```

**macOS:**

```bash
xcode-select --install
brew install cmake ninja ccache openssl clang-format
```

**Windows:**

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
2. Install [CMake](https://cmake.org/download/) ≥ 3.21
3. Install [Git for Windows](https://git-scm.com/download/win)
4. Install [Python 3](https://www.python.org/downloads/windows/)

---

## Cloning and Building

```bash
# Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery

# Initialize submodules
git submodule update --init --recursive

# Create build directory (always build out-of-source)
mkdir build && cd build

# Configure for development (Debug mode + compile_commands.json)
cmake .. \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
cmake --build . --parallel $(nproc)

# Symlink compile_commands.json for IDE/clangd support
cd ..
ln -s build/compile_commands.json compile_commands.json
```

### Useful CMake Flags for Development

| Flag | Description |
|------|-------------|
| `-DCMAKE_BUILD_TYPE=Debug` | Include debug symbols |
| `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` | Generate `compile_commands.json` for IDE intelligence |
| `-DOSQUERY_DISABLE_DATABASE=ON` | Skip RocksDB build (faster for table development) |
| `-DCMAKE_C_COMPILER_LAUNCHER=ccache` | Enable ccache for faster incremental builds |
| `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache` | Enable ccache for C++ compilation |

---

## Project Structure

```text
osquery/
├── osquery/           # Core osquery C++ source (C++17)
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
├── plugins/           # Config, logger, database, distributed plugins
├── libraries/         # Vendored third-party libraries
├── tools/             # Code generation and CI scripts
├── external/          # Extension SDK examples
└── tests/             # Integration tests
```

---

## Code Style

### C++ Standards

- The codebase uses **C++17**
- Follow the existing code style in the file you are editing
- Use `clang-format` to format all C++ files before submitting

```bash
# Format a file in-place
clang-format -i openframe/openframe_encryption_service.cpp

# Check formatting without modifying
clang-format --dry-run --Werror openframe/openframe_encryption_service.cpp
```

### Naming Conventions

| Construct | Convention | Example |
|-----------|-----------|---------|
| Classes | PascalCase | `OpenframeEncryptionService` |
| Methods | camelCase | `extractToken()` |
| Variables | snake_case | `token_path` |
| Constants | UPPER_SNAKE_CASE | `MAX_RETRY_COUNT` |
| Files | snake_case | `openframe_token_extractor.cpp` |

### General Guidelines

- Write self-documenting code; add comments for non-obvious logic
- Keep functions focused on a single responsibility
- Prefer `const` and RAII patterns
- Do not introduce new dependencies without discussion in the community Slack
- Follow existing patterns in the module you are modifying

---

## Adding New OpenFrame Source Files

When adding a new C++ source file to the `openframe/` directory:

1. Create the `.h` and `.cpp` files
2. Add the source to the relevant `CMakeLists.txt`:

```cmake
target_sources(openframe_lib PRIVATE
  openframe/openframe_new_component.cpp
)
```

3. Re-run CMake and rebuild:

```bash
cd build
cmake ..
cmake --build . --parallel $(nproc)
```

---

## Running Tests

```bash
cd build

# Run all unit tests
ctest --parallel $(nproc) --output-on-failure

# Run a specific test binary
./osquery/osquery_tests --gtest_filter=ConfigTests.*

# Run with verbose output
./osquery/osquery_tests --gtest_filter="*" --gtest_verbose=all
```

### Writing Tests

Tests use **Google Test (gtest)**. When modifying security-sensitive code in `openframe/`, always add unit tests:

```cpp
// Test that decryption fails on tampered ciphertext
TEST(EncryptionServiceTest, TamperedCiphertextThrows) {
    OpenframeEncryptionService svc("your-32-byte-secret-key-here!!");
    EXPECT_THROW(svc.decrypt("tampered_base64_payload"), std::runtime_error);
}
```

---

## Security Guidelines

Security is a top priority for all contributions. Follow these rules strictly:

- **Never log token values** at any severity level
- **Never hardcode secrets** — use environment variable injection or a secrets manager
- **Always validate file paths** before opening them
- **Always join background threads** on shutdown
- **Never disable TLS peer verification** in production code
- Use `OpenframeAuthorizationManagerProvider` to access the authorization manager — direct construction is private by design

### Code Review Security Checklist

Before submitting a PR that touches `openframe/`, `osquery/remote/`, or authentication logic:

- [ ] No secrets or tokens appear in log output
- [ ] Secret keys are not hardcoded in source or tests
- [ ] Encryption errors throw exceptions and do NOT silently return empty strings
- [ ] All file paths are validated before use
- [ ] Background threads are properly joined on shutdown
- [ ] No new unsafe SQL execution paths are introduced
- [ ] TLS peer verification is not disabled

---

## Pull Request Process

1. **Discuss first** — Open a thread in the [OpenMSP Slack](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) before starting significant work
2. **Branch naming** — Use descriptive branch names: `feature/openframe-token-rotation`, `fix/rocksdb-shutdown-hang`
3. **Commit messages** — Write clear, imperative commit messages: `Add AES key rotation support to OpenframeEncryptionService`
4. **Keep PRs focused** — One logical change per PR; avoid mixing refactors with feature work
5. **Run tests** — Ensure `ctest` passes before submitting
6. **Format code** — Run `clang-format` on all modified files
7. **Update documentation** — If you change a public interface or behavior, update the relevant docs

### Commit Message Format

```text
<type>: <short summary>

<optional longer description>
```

Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`, `security`

**Examples:**

```text
feat: add OpenframeTokenRefresher configurable interval

fix: resolve RocksDB file lock during rapid daemon restarts

security: enforce TLS peer verification in all remote HTTP calls

docs: update prerequisites with OpenSSL version requirement
```

---

## Running Locally in Development Mode

Use the in-memory database for rapid iteration to avoid RocksDB file locking:

```bash
./build/osquery/osqueryd \
  --config_path=/tmp/dev_osquery.conf \
  --ephemeral=true \
  --disable_database=true \
  --verbose
```

---

## Debugging

### LLDB (macOS / Linux)

```bash
lldb ./build/osquery/osqueryi
(lldb) breakpoint set --name OpenframeEncryptionService::decrypt
(lldb) run --verbose
```

### GDB (Linux)

```bash
gdb ./build/osquery/osqueryi
(gdb) break OpenframeTokenExtractor::extractToken
(gdb) run --verbose
```

---

## Environment Variables

| Variable | Purpose | Notes |
|----------|---------|-------|
| `CC` | C compiler path | e.g., `/usr/bin/clang-13` |
| `CXX` | C++ compiler path | e.g., `/usr/bin/clang++-13` |
| `CCACHE_DIR` | ccache storage directory | e.g., `$HOME/.ccache` |
| `OPENFRAME_TOKEN_PATH` | Path to encrypted token file | For OpenFrame integration |
| `OPENFRAME_SECRET_KEY` | AES-256 key for token decryption | Inject from secrets manager; never hardcode |

---

## Getting Help

All development discussions happen in the [OpenMSP Slack community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA). This is the fastest way to get answers, discuss architecture decisions, or ask about contribution guidelines.

- **OpenMSP Community**: [https://www.openmsp.ai/](https://www.openmsp.ai/)
- **Flamingo**: [https://flamingo.run](https://flamingo.run)
- **OpenFrame**: [https://openframe.ai](https://openframe.ai)

---

<div align="center">
  Built with 💛 by the <a href="https://www.flamingo.run/about"><b>Flamingo</b></a> team
</div>
