# Contributing to osquery — OpenFrame Edition

Thank you for your interest in contributing to osquery with the OpenFrame integration! This document covers everything you need: code style, branching conventions, commit messages, pull requests, and the review process.

---

## 💬 Community First

All discussion, questions, bug reports, and feature requests are handled via the **OpenMSP Slack** community. We do not use GitHub Issues or GitHub Discussions.

- **Slack:** [Join OpenMSP](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- **Community site:** [openmsp.ai](https://www.openmsp.ai/)

**Before opening a pull request**, please discuss the change in Slack to ensure it aligns with the project roadmap.

---

## 🛠️ Development Setup

### Hardware Requirements

| Tier | RAM | CPU Cores | Disk Space |
|------|-----|-----------|------------|
| **Minimum** | 24 GB | 6 cores | 50 GB |
| **Recommended** | 32 GB | 12 cores | 100 GB |

### Quick Developer Commands

```bash
# Clone the repository
git clone https://github.com/flamingo-stack/osquery.git
cd osquery
git submodule update --init --recursive

# Configure build (Debug mode with tests)
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build everything
cmake --build build -j$(nproc)

# Run tests
cd build && ctest --output-on-failure

# Format code
cmake --build build --target clang-format
```

See [Development Documentation](./docs/README.md) for full environment setup, local development, testing, and security guides.

---

## ✏️ Code Style and Conventions

The project follows the **Google C++ Style Guide** with osquery-specific modifications. `clang-format` enforces formatting — configuration is in `.clang-format` at the project root.

**Before every commit, format your changes:**

```bash
# Format all staged C++ files
git diff --cached --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i

# Format all modified files
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i
```

### Key Conventions

| Convention | Rule |
|------------|------|
| **Namespacing** | All code lives in the `osquery` namespace |
| **File naming** | `snake_case.cpp` / `snake_case.h` |
| **Class naming** | `PascalCase` |
| **Function naming** | `camelCase` |
| **Constant naming** | `kConstantName` prefix |
| **Member variables** | Trailing underscore: `member_` |
| **Include guards** | `#pragma once` (not `#ifndef` guards) |
| **Smart pointers** | Prefer `std::shared_ptr` and `std::unique_ptr` over raw pointers |
| **Error handling** | Use `osquery::Status` or `osquery::Expected<T, Error>` |

### Example: Correct Style

```cpp
#pragma once

#include <memory>
#include <string>

namespace osquery {

/// Brief description of what this class does.
class MyNewFeature {
 public:
  explicit MyNewFeature(std::shared_ptr<SomeDependency> dep);
  ~MyNewFeature();

  /// Returns the current feature value, or an error status.
  Expected<std::string, MyError> getValue() const;

 private:
  std::shared_ptr<SomeDependency> dependency_;
  std::string cached_value_;
};

} // namespace osquery
```

---

## 🌿 Branch Naming

| Branch Type | Pattern | Example |
|-------------|---------|---------|
| Feature | `feature/<description>` | `feature/openframe-token-rotation` |
| Bug fix | `fix/<description>` | `fix/sql-authorizer-pragma-list` |
| Documentation | `docs/<description>` | `docs/extension-api-guide` |
| Refactor | `refactor/<description>` | `refactor/database-interface` |
| Hotfix | `hotfix/<description>` | `hotfix/token-refresher-crash` |

```bash
# Create a feature branch from main
git checkout main
git pull origin main
git checkout -b feature/my-new-virtual-table
```

---

## 📝 Commit Message Format

Use the **Conventional Commits** specification:

```text
<type>(<scope>): <short summary>

[Optional body — explain WHY, not WHAT]

[Optional footer — breaking changes, references]
```

### Types

| Type | When to Use |
|------|-------------|
| `feat` | New feature or virtual table |
| `fix` | Bug fix |
| `docs` | Documentation changes only |
| `refactor` | Code change that neither fixes a bug nor adds a feature |
| `test` | Adding or modifying tests |
| `perf` | Performance improvement |
| `ci` | CI/CD configuration changes |
| `chore` | Build system, dependency updates |

### Examples

```text
feat(sql): add constraint pushdown for inet_diag table
fix(events): prevent duplicate subscription on config reload
docs(openframe): add token rotation guide
refactor(database): extract IDatabaseInterface for testability
test(core): add flag override tests for daemon mode
perf(events): use batch DB writes in EventSubscriberPlugin
```

### Full Commit Example

```text
feat(openframe): add configurable token refresh interval

Previously the token refresher used a hardcoded 5-minute interval.
This change reads the interval from the --openframe_refresh_interval
flag, allowing operators to tune refresh frequency based on their
token expiry policies.

Closes: FLAMINGO-1234
```

---

## 🔄 Pull Request Process

### Before Opening a PR

1. **Discuss in Slack** — confirm the change is wanted before investing time
2. **Branch from `main`** — never commit directly to `main`
3. **Run tests** — all tests must pass locally
4. **Format code** — run `clang-format` on all changed files
5. **Update docs** — if you add a new module or change behavior, update or add inline docs

```bash
# Full pre-PR checklist
git checkout -b feature/my-change
# ... make changes ...

# 1. Format
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i

# 2. Build
cmake --build build -j$(nproc)

# 3. Test
cd build && ctest --output-on-failure

# 4. Commit
git add -A
git commit -m "feat(sql): add my new virtual table"

# 5. Push
git push origin feature/my-change
```

### PR Description Template

```markdown
## Summary
Brief description of what this PR does and why.

## Changes
- Added `MyNewTable` virtual table in `osquery/tables/system/`
- Added unit tests in `tests/integration/tables/my_new_table.cpp`
- Updated CMakeLists.txt to include new table

## Testing
- [ ] All existing tests pass (`ctest --output-on-failure`)
- [ ] New tests added for new functionality
- [ ] Manually tested on Linux x86_64
- [ ] Manually tested on macOS (if applicable)

## Breaking Changes
None / Describe any breaking changes here.
```

---

## ✅ Code Review Checklist

Reviewers should verify:

### Correctness
- [ ] Logic is correct and handles edge cases
- [ ] Error paths return appropriate `Status` or `Expected<>` values
- [ ] No resource leaks (file handles, DB connections, threads)

### Security
- [ ] No hardcoded secrets or credentials
- [ ] Query constraints from `QueryContext` are validated before use
- [ ] New network code uses the `Remote HTTP Client` module
- [ ] Thread-shared state is properly synchronized

### Style and Quality
- [ ] Follows naming conventions (see above)
- [ ] `clang-format` applied to all changed files
- [ ] Public APIs have doc comments
- [ ] No dead code or commented-out blocks

### Tests
- [ ] New functionality has tests
- [ ] Tests are independent (use ephemeral DB, no shared state)
- [ ] Tests cover both happy path and error paths

### Documentation
- [ ] Inline docs updated if behavior changes
- [ ] New modules have doc comments on public API

---

## 🗃️ Adding a New Virtual Table

Virtual tables are the most common contribution type. Follow this pattern:

1. **Create the table spec** in `osquery/tables/` (appropriate subdirectory)
2. **Implement `generate()`** — use `QueryContext` for constraint pushdown
3. **Register the table** in the appropriate `CMakeLists.txt`
4. **Add an integration test** in `tests/integration/tables/`
5. **Test on all target platforms** where the table is supported

```cpp
// osquery/tables/system/linux/my_new_table.cpp
#include <osquery/core/tables.h>
#include <osquery/logger/logger.h>

namespace osquery {
namespace tables {

TableRows genMyNewTable(QueryContext& context) {
  TableRows results;

  // Validate constraints before use
  auto paths = context.constraints["path"].getAll(EQUALS);
  for (const auto& path : paths) {
    if (path.empty()) continue;

    Row r;
    r["path"] = path;
    r["size"] = INTEGER(0); // populate from OS API
    results.push_back(r);
  }

  return results;
}

} // namespace tables
} // namespace osquery
```

---

## 🔒 Security Guidelines

Before submitting a pull request, verify:

- [ ] No secrets, tokens, or API keys in source code or test fixtures
- [ ] New virtual tables sanitize all `QueryContext` constraints before use
- [ ] New network-facing code uses the `Remote HTTP Client` module (not raw sockets)
- [ ] All file paths received from queries are validated
- [ ] New config parameters have documented size/depth limits if they accept user data
- [ ] Thread-shared state uses appropriate synchronization primitives

To report a security vulnerability, **do not open a GitHub Issue**. Contact the team via the [OpenMSP Slack](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA).

---

## 🧪 CI Test Matrix

Tests are run across the following configurations:

| Platform | Compiler | Build Type |
|----------|----------|------------|
| Linux x86_64 | GCC 11 | Debug + Release |
| Linux aarch64 | GCC 11 | Debug |
| macOS x86_64 | AppleClang | Debug + Release |
| macOS aarch64 | AppleClang | Debug + Release |
| Windows x86_64 | MSVC 2022 | Debug + Release |

---

## 🆘 Getting Help

Stuck on something? The community is here to help:

- **Slack:** [OpenMSP Community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- **Community:** [openmsp.ai](https://www.openmsp.ai/)
- **Platform:** [flamingo.run](https://flamingo.run) | [openframe.ai](https://openframe.ai)

---

<div align="center">
  Built with 💛 by the <a href="https://www.flamingo.run/about"><b>Flamingo</b></a> team
</div>
