# Prerequisites

Before building or running osquery from source, ensure your environment satisfies the requirements below.

---

## Hardware Requirements

| Tier | RAM | CPU Cores | Disk |
|---|---|---|---|
| **Minimum** | 24 GB | 6 cores | 50 GB |
| **Recommended** | 32 GB | 12 cores | 100 GB |

> The build process compiles a large number of bundled libraries (RocksDB, Thrift, Boost, OpenSSL, etc.) in parallel. Insufficient RAM or CPU will significantly increase build times.

---

## Supported Operating Systems

| Platform | Architecture | Status |
|---|---|---|
| Linux (Ubuntu 20.04+, Fedora, CentOS 8+) | x86_64, aarch64 | ✅ Fully supported |
| macOS 11+ | x86_64, Apple Silicon (aarch64) | ✅ Fully supported |
| Windows 10 / Server 2016+ | x86_64, aarch64 | ✅ Fully supported |

---

## Required Software

### Build Tools

| Tool | Minimum Version | Notes |
|---|---|---|
| CMake | 3.21+ | Build system generator |
| Git | 2.x | Source control and submodule management |
| Python | 3.6+ | Code generation scripts (`tools/codegen/`) |
| Clang / LLVM | 11+ (Linux/macOS) | Primary compiler |
| MSVC | VS 2019+ | Windows builds only |
| Ninja | 1.10+ | Recommended build backend (faster than Make) |

### Linux-Specific Dependencies

These packages are required on Linux before building:

```bash
# Ubuntu / Debian
sudo apt-get update && sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  python3 \
  python3-pip \
  clang \
  llvm \
  ninja-build \
  libstdc++-dev \
  lzma-dev \
  zlib1g-dev \
  libssl-dev

# Fedora / RHEL / CentOS
sudo dnf install -y \
  gcc-c++ \
  cmake \
  git \
  python3 \
  python3-pip \
  clang \
  llvm \
  ninja-build \
  openssl-devel \
  zlib-devel \
  xz-devel
```

### macOS-Specific Dependencies

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install build dependencies
brew install cmake ninja python3 llvm
```

### Windows-Specific Dependencies

1. Install **Visual Studio 2019 or 2022** with the "Desktop Development with C++" workload.
2. Install **CMake** (3.21+) — available from https://cmake.org/download/.
3. Install **Python 3.6+** — available from https://www.python.org/downloads/.
4. Install **Git for Windows** — available from https://git-scm.com/.
5. Install **Ninja** — download from https://ninja-build.org/ and add to `PATH`.

For AMD64 targets on Windows, the OpenFrame CLI installer can be obtained from:

```text
https://github.com/flamingo-stack/openframe-cli/releases/latest/download/openframe-cli_windows_amd64.zip
```

---

## Python Packages (for tooling)

The code generation and test scripts require these Python packages:

```bash
pip3 install jinja2 six future
```

For integration tests:

```bash
pip3 install pexpect psutil timeout-decorator
```

---

## Environment Variables

| Variable | Purpose | Example |
|---|---|---|
| `OSQUERY_WORKER` | Set by the daemon to identify worker processes | Set automatically by `osqueryd` |
| `CC` | C compiler override | `clang` |
| `CXX` | C++ compiler override | `clang++` |

> You generally do not need to set any environment variables manually. The CMake build system auto-detects compilers.

---

## Account / Access Requirements

| Resource | Requirement |
|---|---|
| GitHub | Read access to [flamingo-stack/osquery](https://github.com/flamingo-stack/osquery) |
| Sudo / Admin | Required for installing system packages and running `osqueryd` as a service |
| TLS endpoint (optional) | Required for distributed querying and remote logging |

---

## Verification Commands

Run these commands to verify your environment is ready:

```bash
# Check CMake version (need 3.21+)
cmake --version

# Check Git version
git --version

# Check Python version (need 3.6+)
python3 --version

# Check Clang version (Linux/macOS)
clang --version

# Check Ninja version
ninja --version

# Confirm Python packages
python3 -c "import jinja2; print('jinja2 ok')"
python3 -c "import six; print('six ok')"
```

Expected output (versions will vary):

```text
cmake version 3.25.1
git version 2.40.1
Python 3.11.4
clang version 15.0.7
1.11.1
jinja2 ok
six ok
```

---

## Community Support

Questions and help are available in the **OpenMSP Slack community**:

https://www.openmsp.ai/
