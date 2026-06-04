# Development Environment Setup

This guide covers IDE recommendations, required development tools, and editor configuration for working on osquery with the OpenFrame integration.

---

## IDE Recommendations

### Visual Studio Code (Recommended — All Platforms)

VS Code with the CMake and C++ extensions provides the best cross-platform development experience.

**Required Extensions:**

| Extension | Publisher | Purpose |
|-----------|-----------|---------|
| C/C++ | Microsoft | IntelliSense, debugging |
| CMake Tools | Microsoft | CMake configure/build integration |
| CMake | twxs | CMake syntax highlighting |
| clangd | LLVM | Advanced code completion and navigation |
| GitLens | GitKraken | Enhanced Git integration |

Install all at once:

```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
code --install-extension llvm-vs-code-extensions.vscode-clangd
code --install-extension eamodio.gitlens
```

**Recommended VS Code settings (`.vscode/settings.json`):**

```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureArgs": [
    "-DCMAKE_BUILD_TYPE=Debug",
    "-DOSQUERY_BUILD_TESTS=ON",
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
  ],
  "cmake.generator": "Ninja",
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "editor.formatOnSave": true,
  "editor.tabSize": 2,
  "files.trimTrailingWhitespace": true
}
```

---

### CLion (Recommended — Linux / macOS)

JetBrains CLion has native CMake support and powerful C++ refactoring tools.

**Setup steps:**

1. Open the project root (where `CMakeLists.txt` is located)
2. CLion auto-detects CMake configuration
3. Set CMake options in **Settings → Build → CMake**:

```text
-DCMAKE_BUILD_TYPE=Debug
-DOSQUERY_BUILD_TESTS=ON
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**Recommended plugins:**

- **File Watchers** — auto-format on save
- **GitToolBox** — enhanced Git annotations

---

### Xcode (macOS Only)

For macOS-specific development or debugging macOS-specific event publishers:

```bash
# Generate Xcode project
cmake -S . -B build-xcode -G Xcode \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOSQUERY_BUILD_TESTS=ON

# Open in Xcode
open build-xcode/osquery.xcodeproj
```

---

### Visual Studio 2022 (Windows)

Visual Studio 2022 has native CMake support. Open the folder directly (File → Open → Folder) and VS will detect `CMakeLists.txt` automatically.

---

## Required Development Tools

### Core Toolchain

```bash
# Linux (Ubuntu/Debian)
sudo apt-get install -y \
  clang-14 lldb-14 clang-format-14 clang-tidy-14 \
  cmake ninja-build ccache \
  valgrind gdb

# macOS
brew install llvm cmake ninja ccache
```

### ccache — Compiler Cache (Strongly Recommended)

ccache dramatically speeds up incremental builds. Enable it in CMake:

```bash
cmake -S . -B build \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_BUILD_TYPE=Debug
```

### compile_commands.json — IntelliSense Database

Always generate this for IDE support:

```bash
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# Optionally symlink to the root for clangd
ln -sf build/compile_commands.json ./compile_commands.json
```

---

## Environment Variables for Development

| Variable | Purpose | Example Value |
|----------|---------|---------------|
| `CC` | C compiler override | `clang-14` |
| `CXX` | C++ compiler override | `clang++-14` |
| `CCACHE_DIR` | ccache storage directory | `/tmp/ccache` |
| `OSQUERY_BUILD_TESTS` | Enable test targets | `ON` |
| `CMAKE_BUILD_TYPE` | Build mode | `Debug` or `Release` |

Set in your shell profile (`~/.bashrc`, `~/.zshrc`):

```bash
export CC=clang-14
export CXX=clang++-14
export CCACHE_DIR=/tmp/osquery-ccache
```

---

## Code Formatting

The project uses **clang-format** for C++ code style enforcement. Configuration is in `.clang-format` at the project root.

```bash
# Format all modified files
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i

# Or use the CMake target (if configured)
cmake --build build --target clang-format
```

**Editor integration** — VS Code with the C++ extension will auto-format on save if you set:

```json
{
  "editor.formatOnSave": true,
  "C_Cpp.clang_format_fallbackStyle": "file"
}
```

---

## Debug Symbols and Sanitizers

For debugging crashes or memory issues:

```bash
# Debug build with AddressSanitizer
cmake -S . -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_LINKER_FLAGS="-fsanitize=address"

cmake --build build-asan --target osqueryi -j$(nproc)
```

```bash
# Debug build with ThreadSanitizer
cmake -S . -B build-tsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread"
```

---

## GDB / LLDB Quick Reference

```bash
# Debug with GDB (Linux)
gdb ./build/osqueryi
(gdb) run "SELECT * FROM processes;"
(gdb) bt   # print backtrace

# Debug with LLDB (macOS)
lldb ./build/osqueryi
(lldb) run "SELECT * FROM processes;"
(lldb) bt   # print backtrace
```

---

## Checking Your Environment

```bash
# Verify clang-format is available
clang-format --version

# Verify ccache is working
ccache --version
ccache --show-stats

# Verify compile_commands.json was generated
ls -la build/compile_commands.json
```
