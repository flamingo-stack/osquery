# Development Environment Setup

This guide covers IDE recommendations, editor configuration, compiler setup, and useful tooling for developing on the osquery codebase.

---

## Recommended IDEs

### Visual Studio Code (Cross-Platform)

VS Code is the most widely used editor for osquery development across all three platforms.

**Required extensions:**

| Extension | Publisher | Purpose |
|---|---|---|
| C/C++ | Microsoft | IntelliSense, debugging, formatting |
| CMake Tools | Microsoft | CMake integration, build and test from sidebar |
| CMake Language Support | twxs | Syntax highlighting for CMakeLists.txt |
| clangd | LLVM | Fast code completion, diagnostics, go-to-definition |
| GitLens | GitKraken | Enhanced Git history and blame |

**Installing extensions:**

```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
code --install-extension llvm-vs-code-extensions.vscode-clangd
code --install-extension eamodio.gitlens
```

**Workspace settings (`.vscode/settings.json`):**

```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "cmake.buildType": "RelWithDebInfo",
  "clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/build",
    "--clang-tidy",
    "--background-index"
  ],
  "editor.formatOnSave": true,
  "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
  "C_Cpp.intelliSenseEngine": "disabled"
}
```

> Set `C_Cpp.intelliSenseEngine` to `"disabled"` when using clangd to avoid conflicts.

---

### CLion (Cross-Platform)

JetBrains CLion has excellent CMake and C++ support built-in.

**Configuration:**
1. Open the repository root — CLion auto-detects the top-level `CMakeLists.txt`.
2. Go to **File → Settings → Build, Execution, Deployment → CMake**.
3. Set **Build type** to `RelWithDebInfo`.
4. Set **CMake options** to `-G Ninja`.
5. Set **Build directory** to `build`.

---

### Xcode (macOS Only)

Generate an Xcode project from the CMake configuration:

```bash
cmake -B build-xcode -S . -G Xcode \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
open build-xcode/osquery.xcodeproj
```

---

### Visual Studio (Windows Only)

Generate a Visual Studio solution:

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
start build\osquery.sln
```

---

## Compiler Setup

osquery uses **Clang** as the primary compiler on Linux and macOS, and **MSVC** on Windows.

### Linux — Install Clang

```bash
# Ubuntu / Debian
sudo apt-get install -y clang-15 llvm-15 lld-15

# Set as default (optional)
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-15 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-15 100
```

### macOS — Use Homebrew LLVM

```bash
brew install llvm

# Add to your shell profile:
export PATH="$(brew --prefix llvm)/bin:$PATH"
export CC=clang
export CXX=clang++
```

### Specifying Compilers in CMake

```bash
cmake -B build -S . -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++
```

---

## Compile Commands Database

osquery's CMake configuration generates a `compile_commands.json` file automatically when using Ninja or Makefiles. This enables clangd and other tools to provide accurate IntelliSense:

```bash
cmake -B build -S . -G Ninja \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Symlink to repo root for clangd auto-discovery
ln -sf build/compile_commands.json compile_commands.json
```

---

## Code Formatting

osquery uses **clang-format** for consistent C++ formatting.

```bash
# Install clang-format
sudo apt-get install -y clang-format-15   # Linux
brew install clang-format                  # macOS

# Format a single file
clang-format -i osquery/core/init.cpp

# Check formatting without modifying (for CI)
tools/formatting/format-check.sh
```

The `.clang-format` configuration in the repository root defines the style rules.

---

## Static Analysis

```bash
# Run clang-tidy on a file
clang-tidy osquery/core/init.cpp \
  -p build/compile_commands.json

# Run with fixes applied
clang-tidy -fix osquery/core/init.cpp \
  -p build/compile_commands.json
```

---

## Development Environment Variables

| Variable | Recommended Value | Description |
|---|---|---|
| `CC` | `clang` | C compiler |
| `CXX` | `clang++` | C++ compiler |
| `NINJA_STATUS` | `[%f/%t %e] ` | Show build progress with elapsed time |
| `CLICOLOR_FORCE` | `1` | Force colored compiler output |

Add to your shell profile (`~/.bashrc`, `~/.zshrc`):

```bash
export CC=clang
export CXX=clang++
export NINJA_STATUS="[%f/%t %e] "
```

---

## Useful Aliases

```bash
# Add to ~/.bashrc or ~/.zshrc
alias osquery-build='cmake --build build -j$(nproc)'
alias osquery-configure='cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo'
alias osqueryi='./build/osquery/osqueryi'
alias osqueryd='./build/osquery/osqueryd'
```

---

## Python Environment for Tooling

osquery's code generation and test tooling requires Python 3:

```bash
# Create a virtual environment
python3 -m venv .venv
source .venv/bin/activate   # Linux/macOS
.venv\Scripts\activate       # Windows

# Install tooling dependencies
pip install jinja2 six future pexpect psutil timeout-decorator
```

---

## Community

For questions about development setup, join the **OpenMSP Slack**:

https://www.openmsp.ai/
