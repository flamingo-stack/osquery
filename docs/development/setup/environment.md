# Development Environment Setup

This guide covers recommended IDE configurations, development tools, editor extensions, and environment variables for contributing to osquery with OpenFrame.

---

## Recommended IDEs

### Visual Studio Code (All Platforms)

VS Code with the C++ extension pack is the recommended cross-platform IDE for osquery development.

**Required Extensions:**

| Extension | ID | Purpose |
|---|---|---|
| C/C++ | `ms-vscode.cpptools` | IntelliSense, debugging, syntax highlighting |
| CMake Tools | `ms-vscode.cmake-tools` | CMake integration, build configuration |
| CMake | `twxs.cmake` | CMake syntax highlighting |
| clangd | `llvm-vs-code-extensions.vscode-clangd` | Fast code navigation, diagnostics |

**Optional but Recommended:**

| Extension | ID | Purpose |
|---|---|---|
| GitLens | `eamodio.gitlens` | Enhanced Git history and annotations |
| Clang-Format | `xaver.clang-format` | Auto-formatting on save |
| Error Lens | `usernamehakki.error-lens` | Inline error display |

**Install all at once:**

```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
code --install-extension llvm-vs-code-extensions.vscode-clangd
code --install-extension eamodio.gitlens
```

**Recommended `.vscode/settings.json`:**

```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "cmake.buildType": "Debug",
  "editor.formatOnSave": true,
  "clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/build",
    "--header-insertion=iwyu",
    "--clang-tidy"
  ],
  "C_Cpp.intelliSenseEngine": "disabled"
}
```

> When using `clangd`, disable the built-in IntelliSense engine to avoid conflicts.

---

### CLion (JetBrains)

CLion provides first-class CMake support. Open the project root and CLion will auto-detect the `CMakeLists.txt`.

**Recommended Settings:**

- Set the CMake build directory to `build/`
- Enable `clang-format` in **Settings → Editor → Code Style → C/C++**
- Use the built-in CMake tab for build configuration

---

### Xcode (macOS Only)

Generate an Xcode project from CMake:

```bash
cmake -S . -B build-xcode -G Xcode
open build-xcode/osquery.xcodeproj
```

---

## Development Tools

Install these tools before starting development:

### All Platforms

```bash
# Python tools for code generation
pip3 install jinja2 pexpect six
```

### Linux

```bash
sudo apt-get install -y \
  clang-format \
  clang-tidy \
  ccache \
  valgrind \
  gdb
```

### macOS

```bash
brew install \
  clang-format \
  ccache \
  llvm
```

### Windows

- Install [LLVM](https://releases.llvm.org/) for `clang-format` and `clang-tidy`
- Configure `PATH` to include the LLVM `bin` directory

---

## Setting Up ccache (Highly Recommended)

`ccache` dramatically speeds up incremental rebuilds:

```bash
# Install ccache
sudo apt-get install ccache   # Linux
brew install ccache            # macOS

# Enable in CMake
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

---

## Generate compile_commands.json for clangd

For full IntelliSense support with `clangd`, generate the compilation database:

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Link to project root (required by clangd)
ln -sf build/compile_commands.json compile_commands.json
```

---

## Environment Variables for Development

| Variable | Purpose | Example |
|---|---|---|
| `OSQUERY_EXTENSIONS_SOCKET` | Extension manager socket path | `/tmp/osquery.em` |
| `OSQUERY_CONFIG_PATH` | Override default config location | `./test.conf` |
| `OSQUERY_DB_PATH` | Custom database path for development | `/tmp/osquery-dev.db` |
| `OPENSSL_ROOT_DIR` | OpenSSL installation path (Windows/macOS) | `/usr/local/opt/openssl` |

Set in your shell profile or export before building:

```bash
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3"
```

On Windows:

```bash
set OPENSSL_ROOT_DIR=C:\OpenSSL-Win64
```

---

## Code Formatting

osquery enforces formatting with `clang-format`. The configuration is in `.clang-format` at the repository root.

```bash
# Format a single file
clang-format -i osquery/core/init.cpp

# Check formatting without modifying
clang-format --dry-run --Werror osquery/core/init.cpp

# Format all C++ files in a directory
find osquery/core -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

The CI pipeline enforces formatting. Run format checks before submitting PRs.

---

## Next Steps

Once your environment is set up, proceed to the [Local Development Guide](local-development.md) for instructions on building, running, and debugging osquery locally.
