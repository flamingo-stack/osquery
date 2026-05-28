# Prerequisites

Before building or deploying osquery with OpenFrame, ensure your environment meets the requirements below.

---

## System Requirements

| Tier | RAM | CPU Cores | Disk Space |
|---|---|---|---|
| **Minimum** | 24 GB | 6 cores | 50 GB |
| **Recommended** | 32 GB | 12 cores | 100 GB |

> Building osquery from source is resource-intensive. The recommended configuration significantly reduces build times and prevents out-of-memory failures during compilation.

---

## Supported Operating Systems

| Platform | Architecture | Notes |
|---|---|---|
| Linux | x86_64, aarch64 | Ubuntu 20.04+, RHEL 8+, Debian 11+ |
| macOS | x86_64, aarch64 (Apple Silicon) | macOS 12+ recommended |
| Windows | x86_64, aarch64 | Windows 10/11, Server 2019+ |

---

## Required Software

| Tool | Minimum Version | Purpose |
|---|---|---|
| **CMake** | 3.21+ | Build system generator |
| **Python** | 3.8+ | Code generation, tooling scripts |
| **Git** | 2.x | Source control |
| **C++ Compiler** | GCC 9+ / Clang 10+ / MSVC 2019+ | Compiling C++17 sources |
| **Ninja** | 1.10+ | Fast parallel builds (recommended) |
| **OpenSSL** | 1.1.1+ | TLS support and AES-256-GCM encryption (OpenFrame) |

---

## Optional but Recommended

| Tool | Purpose |
|---|---|
| **ccache** | Speeds up incremental C++ builds significantly |
| **clang-format** | Code formatting (enforced by CI) |
| **Docker** | Isolated build environments for Linux targets |

---

## Platform-Specific Notes

### Linux

On Debian/Ubuntu, install build essentials:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  python3 \
  python3-pip \
  git \
  openssl \
  libssl-dev \
  clang-format
```

On RHEL/CentOS/Fedora:

```bash
sudo dnf install -y \
  gcc-c++ \
  cmake \
  ninja-build \
  python3 \
  git \
  openssl-devel \
  clang
```

### macOS

Install Xcode Command Line Tools and Homebrew tools:

```bash
xcode-select --install
brew install cmake ninja python3 openssl git
```

### Windows

1. Install [Visual Studio 2019 or 2022](https://visualstudio.microsoft.com/) with the **Desktop development with C++** workload
2. Install [CMake](https://cmake.org/download/) 3.21+
3. Install [Git for Windows](https://git-scm.com/download/win)
4. Install [Python 3.8+](https://www.python.org/downloads/windows/)
5. Optionally install [Ninja](https://github.com/ninja-build/ninja/releases)

---

## OpenFrame Platform Requirements

If you are connecting to the [OpenFrame platform](https://openframe.ai), you also need:

| Requirement | Description |
|---|---|
| **OpenFrame Account** | Active account on the Flamingo/OpenFrame platform |
| **Network Access** | Outbound HTTPS (port 443) to OpenFrame services |
| **AES-256 Key** | Symmetric secret key for the `OpenframeEncryptionService` |
| **JWT Token** | Authorization token managed by `OpenframeAuthorizationManager` |

Refer to your environment configuration and OpenFrame administrator for credential details.

---

## Environment Variables

The following environment variables may be required depending on your deployment:

| Variable | Purpose |
|---|---|
| `OSQUERY_FLAGS_FILE` | Path to a flagfile for osquery runtime configuration |
| `OSQUERY_EXTENSIONS_SOCKET` | Path to the Thrift extension manager socket |
| `OSQUERY_CONFIG_PATH` | Custom configuration file path (overrides default) |

Set variables using your shell profile or deployment tooling. For example:

```bash
export OSQUERY_FLAGS_FILE="/etc/osquery/osquery.flags"
export OSQUERY_EXTENSIONS_SOCKET="/var/osquery/osquery.em"
```

---

## Verifying Your Environment

Run the following checks to confirm your build environment is ready:

```bash
# Check CMake version
cmake --version

# Check C++ compiler
c++ --version

# Check Python
python3 --version

# Check Git
git --version

# Check OpenSSL
openssl version

# Check Ninja (if installed)
ninja --version
```

All tools should report versions meeting the minimums in the table above.

---

## Next Steps

Once prerequisites are confirmed, proceed to the [Quick Start Guide](quick-start.md) to clone, build, and run osquery.
