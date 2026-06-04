# Development Environment Setup

This guide covers how to configure your development environment for working on osquery (OpenFrame-enhanced), including IDE setup, recommended tools, and required extensions.

---

## Recommended IDE: CLion or VS Code

### Option A — CLion (Recommended for C++)

[CLion](https://www.jetbrains.com/clion/) provides native CMake integration and is the most productive IDE for this C++17 codebase.

**Setup steps:**

1. Install CLion (commercial license or free trial).
2. Open the repository root directory — CLion auto-detects `CMakeLists.txt`.
3. Configure the CMake profile:
   - Navigate to **Settings → Build, Execution, Deployment → CMake**
   - Add a profile: `Debug` with generator `Ninja` or `Unix Makefiles`
   - Set CMake options:

```text
-DCMAKE_C_COMPILER=clang
-DCMAKE_CXX_COMPILER=clang++
-DCMAKE_BUILD_TYPE=Debug
```

4. Enable **clangd** for code intelligence under **Settings → Languages & Frameworks → C/C++ → Clangd**.

---

### Option B — Visual Studio Code

VS Code works well with the following extensions installed:

| Extension | ID | Purpose |
|-----------|-----|---------|
| **C/C++ Extension Pack** | `ms-vscode.cpptools-extension-pack` | C++ IntelliSense, debugging |
| **CMake Tools** | `ms-vscode.cmake-tools` | CMake integration |
| **clangd** | `llvm-vs-code-extensions.vscode-clangd` | Fast code intelligence via clang |
| **CodeLLDB** | `vadimcn.vscode-lldb` | LLDB debugger integration |
| **GitLens** | `eamodio.gitlens` | Enhanced Git integration |

**VS Code workspace settings** (create `.vscode/settings.json`):

```json
{
  "cmake.configureArgs": [
    "-DCMAKE_C_COMPILER=clang",
    "-DCMAKE_CXX_COMPILER=clang++"
  ],
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "clangd.arguments": [
    "--compile-commands-dir=${workspaceFolder}/build",
    "--background-index",
    "--clang-tidy"
  ],
  "C_Cpp.intelliSenseEngine": "disabled"
}
```

---

## Required Development Tools

### Linux (Ubuntu/Debian)

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
  ccache
```

### macOS

```bash
# Install Xcode command-line tools
xcode-select --install

# Install Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install tools
brew install cmake ninja ccache openssl
```

### Windows

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with:
   - **Desktop development with C++** workload
   - **Windows 11 SDK**
   - **LLVM/Clang compiler** (optional, but recommended)
2. Install [CMake](https://cmake.org/download/) ≥ 3.21.
3. Install [Git for Windows](https://git-scm.com/download/win).
4. Install [Python 3](https://www.python.org/downloads/windows/).

---

## Compiler Configuration

osquery requires **C++17** support. The recommended compiler configuration:

| Platform | Compiler | Version |
|----------|---------|---------|
| Linux | Clang | ≥ 13 |
| macOS | Apple Clang | ≥ 13 (Xcode ≥ 13) |
| Windows | MSVC | Visual Studio 2019+ |

Set compiler explicitly to avoid version mismatches:

```bash
export CC=clang-13
export CXX=clang++-13
```

---

## ccache (Build Cache)

`ccache` significantly speeds up incremental and clean builds by caching compiled objects.

Install and enable:

```bash
# Linux
sudo apt-get install ccache

# macOS
brew install ccache

# Verify
ccache --version
```

Configure in your shell profile:

```bash
export CCACHE_DIR="$HOME/.ccache"
export PATH="/usr/lib/ccache:$PATH"
```

CMake integration:

```bash
cmake .. \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

---

## clang-format (Code Formatting)

The project uses `clang-format` for consistent C++ code style.

Install:

```bash
# Linux
sudo apt-get install clang-format-13

# macOS
brew install clang-format
```

Format a file:

```bash
clang-format -i openframe/openframe_encryption_service.cpp
```

Check formatting without modifying:

```bash
clang-format --dry-run --Werror openframe/openframe_encryption_service.cpp
```

---

## Python Tools (Code Generation)

osquery uses Python scripts for table code generation and CI tasks:

```bash
pip3 install jinja2 six
```

---

## Environment Variables for Development

| Variable | Purpose | Example Value |
|----------|---------|--------------|
| `CC` | C compiler path | `/usr/bin/clang-13` |
| `CXX` | C++ compiler path | `/usr/bin/clang++-13` |
| `CCACHE_DIR` | ccache storage directory | `$HOME/.ccache` |
| `OPENFRAME_TOKEN_PATH` | Path to encrypted token file | `/etc/openframe/token.enc` |
| `OPENFRAME_SECRET_KEY` | AES-256 secret key for token decryption | Refer to your environment configuration |

---

## Generating compile_commands.json

The `compile_commands.json` file is required by clangd for code intelligence:

```bash
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

Then symlink to the project root so clangd and editors find it:

```bash
ln -s build/compile_commands.json compile_commands.json
```

---

## Verifying Your Environment

```bash
# Confirm compiler works
clang++ --std=c++17 -o /dev/null -x c++ /dev/null

# Confirm CMake finds Clang
cmake .. -DCMAKE_CXX_COMPILER=clang++ --trace-source=CMakeLists.txt 2>&1 | head -20

# Confirm OpenSSL is available for the OpenFrame encryption service
pkg-config --libs openssl
```
