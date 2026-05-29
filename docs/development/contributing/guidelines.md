# Contributing Guidelines

Thank you for contributing to osquery with OpenFrame! This guide covers code style, branching, commit messages, and the pull request process.

---

## Community First

All collaboration happens on the **OpenMSP Slack community** — not GitHub Issues or GitHub Discussions.

- 💬 **Join Slack**: [OpenMSP Community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- 🌐 **OpenMSP**: [https://www.openmsp.ai/](https://www.openmsp.ai/)

Before starting significant work, please discuss your changes in the Slack community to align with the team's roadmap.

---

## Code Style and Conventions

### C++ Standards

- Use **C++17** features where appropriate
- Follow the existing code style in each file you modify
- All new code must pass `clang-format` with the repository's `.clang-format` config

### Formatting

osquery enforces `clang-format`. Run it before every commit:

```bash
# Format a single file
clang-format -i path/to/your/file.cpp

# Format all changed files (compared to main branch)
git diff --name-only main | grep -E '\.(cpp|h)$' | xargs clang-format -i

# Check without modifying
clang-format --dry-run --Werror path/to/your/file.cpp
```

### Naming Conventions

| Item | Convention | Example |
|---|---|---|
| Classes | `PascalCase` | `EventSubscriberPlugin` |
| Methods | `camelCase` | `generateRows()` |
| Member variables | `snake_case_` (trailing underscore) | `running_` |
| Constants | `kPascalCase` | `kSQLOpcodes` |
| Macros | `UPPER_SNAKE_CASE` | `DECLARE_FLAG` |
| Namespaces | `snake_case` | `osquery` |
| Files | `snake_case.cpp` / `snake_case.h` | `event_subscriber.cpp` |

### Code Organization

- Keep headers (`*.h`) as minimal as possible — forward declare where possible
- Use the `osquery` namespace for all production code
- Place tests in `tests/` subdirectories alongside the source
- New virtual tables go in `osquery/tables/<category>/`

### Include Order

Follow this include order, with blank lines between groups:

```cpp
// 1. Standard library
#include <memory>
#include <string>
#include <vector>

// 2. Third-party libraries
#include <boost/noncopyable.hpp>
#include <gtest/gtest.h>

// 3. osquery headers
#include "osquery/core/core.h"
#include "osquery/sql/sql.h"

// 4. Local headers (same directory)
#include "my_local_header.h"
```

---

## Branch Naming

| Type | Pattern | Example |
|---|---|---|
| Feature | `feature/<short-description>` | `feature/bpf-socket-events` |
| Bug fix | `fix/<short-description>` | `fix/config-refresh-race` |
| OpenFrame integration | `openframe/<short-description>` | `openframe/token-refresh-retry` |
| Documentation | `docs/<short-description>` | `docs/virtual-table-guide` |
| Refactor | `refactor/<short-description>` | `refactor/sql-authorizer` |
| Test | `test/<short-description>` | `test/events-integration` |
| Release | `release/v<version>` | `release/v5.13.0` |

Always branch from the latest `main`:

```bash
git checkout main
git pull origin main
git checkout -b feature/my-new-feature
```

---

## Commit Message Format

Use the following format for all commits:

```text
<type>(<scope>): <short summary>

<optional body>

<optional footer>
```

### Types

| Type | When to Use |
|---|---|
| `feat` | New feature or capability |
| `fix` | Bug fix |
| `docs` | Documentation changes only |
| `style` | Code formatting, no logic change |
| `refactor` | Code refactoring without behavior change |
| `test` | Adding or fixing tests |
| `perf` | Performance improvement |
| `chore` | Build, CI, tooling changes |
| `openframe` | OpenFrame platform-specific changes |

### Scopes

| Scope | Area |
|---|---|
| `core` | Core init and runtime |
| `sql` | SQL engine and virtual tables |
| `config` | Configuration and packs |
| `events` | Eventing framework |
| `logger` | Logging and observability |
| `db` | Database and storage |
| `distributed` | Distributed querying |
| `extensions` | Extension IPC |
| `http` | Remote HTTP client |
| `openframe` | OpenFrame auth layer |
| `tables` | Virtual table implementations |

### Examples

```text
feat(events): add BPF socket event publisher for Linux

Implements a new BPF-based publisher that captures socket connect/accept
events and exposes them via the bpf_socket_events virtual table.

Closes #1234
```

```text
fix(openframe): handle token refresh failure with exponential backoff

When OpenframeTokenRefresher encounters a network error, it now retries
with exponential backoff instead of immediately stopping the refresh loop.
```

```text
test(config): add pack discovery query unit tests

Adds unit tests for the discovery query evaluation logic in Pack::shouldPackExecute()
covering platform, version, and shard constraints.
```

---

## Pull Request Process

### Before Submitting

- [ ] Branch is up-to-date with `main`
- [ ] All tests pass: `cd build && ctest --output-on-failure`
- [ ] Code is formatted: `clang-format --dry-run --Werror`
- [ ] Copyright headers are present on new files
- [ ] New virtual tables have integration tests in `tests/integration/tables/`
- [ ] OpenFrame-specific changes include updated documentation

### PR Title

Use the same format as commit messages:

```text
feat(sql): add query result caching for repeated virtual table scans
```

### PR Description Template

```markdown
## Summary
<!-- What does this PR do? Why is it needed? -->

## Changes
<!-- Bullet list of changes -->

## Testing
<!-- How was this tested? Which test commands were run? -->

## Platform Support
<!-- Does this affect Linux/macOS/Windows differently? -->

## Checklist
- [ ] Tests pass (ctest)
- [ ] clang-format applied
- [ ] Documentation updated (if applicable)
- [ ] Discussed in OpenMSP Slack (if significant change)
```

---

## Copyright Headers

All new source files must include a copyright header. Use the format already present in existing source files:

```cpp
/**
 * Copyright (c) 2014-present, The osquery authors
 *
 * This source code is licensed in accordance with the terms specified in
 * the LICENSE file found in the root directory of this source tree.
 */
```

The CI script `tools/ci/scripts/check_copyright_headers.py` enforces this on all pull requests.

---

## Review Checklist for Reviewers

When reviewing a PR:

- [ ] Logic is correct and handles error paths
- [ ] No secrets or credentials in source
- [ ] SQL inputs validated (if applicable)
- [ ] Thread safety considered for shared state
- [ ] Platform-specific code properly guarded with `#ifdef`
- [ ] Tests added for new functionality
- [ ] No debug/temporary code left in
- [ ] Commit messages follow format convention
- [ ] Performance impact considered for hot paths (scheduler, SQL engine)

---

## Adding a New Virtual Table

1. Define the table schema in `osquery/tables/<category>/<table_name>.table`
2. Run the code generator:
   ```bash
   python3 tools/codegen/gentable.py osquery/tables/<category>/<table_name>.table
   ```
3. Implement the `generate()` method in `<table_name>.cpp`
4. Register in the CMakefile for your category
5. Add an integration test in `tests/integration/tables/<table_name>.cpp`
6. Test locally:
   ```bash
   cmake --build build --target osqueryi
   ./build/osquery/osqueryi
   osquery> SELECT * FROM <table_name>;
   ```

---

## Code of Conduct

Be respectful, collaborative, and constructive. All contributors are expected to maintain a professional and welcoming environment in both Slack and code reviews.
