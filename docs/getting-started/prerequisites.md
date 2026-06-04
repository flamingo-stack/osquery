# Prerequisites

Before building or running osquery with the OpenFrame integration, ensure your system meets the following requirements.

---

## Hardware Requirements

| Tier | RAM | CPU Cores | Disk Space |
|------|-----|-----------|------------|
| **Minimum** | 24 GB | 6 cores | 50 GB |
| **Recommended** | 32 GB | 12 cores | 100 GB |

> **Note:** The CMake build system compiles a large number of bundled libraries. Builds with fewer than 6 CPU cores will be significantly slower.

---

## Supported Operating Systems

| Platform | Versions |
|----------|---------|
| **Linux** | Ubuntu 20.04+, Debian 11+, CentOS 8+, RHEL 8+, Fedora 36+ |
| **macOS** | 12 (Monterey)+, Apple Silicon (aarch64) and Intel (x86_64) |
| **Windows** | Windows 10/11, Windows Server 2019+ |

---

## Required Software

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| **CMake** | 3.21+ | Build system |
| **Clang** or **GCC** | Clang 12+ / GCC 10+ | C++ compiler (C++17 required) |
| **Git** | 2.30+ | Source control |
| **Python 3** | 3.8+ | Code generation scripts |
| **Bash** | 5.0+ | Build helper scripts |
| **Ninja** | 1.10+ | Recommended build backend (optional) |

### Linux-Specific Dependencies

```bash
# Ubuntu / Debian
sudo apt-get update && sudo apt-get install -y \
  build-essential cmake git python3 python3-pip \
  libssl-dev libgflags-dev libgoogle-glog-dev \
  librocksdb-dev libbz2-dev libsnappy-dev \
  libboost-all-dev liblzma-dev libthrift-dev

# Fedora / RHEL / CentOS
sudo dnf install -y \
  gcc-c++ cmake git python3 \
  openssl-devel gflags-devel glog-devel \
  boost-devel xz-devel
```

### macOS-Specific Dependencies

```bash
# Homebrew is required
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git python3 openssl gflags glog boost thrift
```

### Windows-Specific Dependencies

Download and install the following:

- [Visual Studio 2022](https://visualstudio.microsoft.com/) (with C++ Desktop workload)
- [CMake for Windows](https://cmake.org/download/)
- [Git for Windows](https://git-scm.com/download/win)
- [Python 3 for Windows](https://www.python.org/downloads/windows/)

---

## OpenFrame Integration Requirements

If you are building with OpenFrame mode enabled, you additionally need:

| Requirement | Details |
|-------------|---------|
| **OpenFrame Account** | An active account at [openframe.ai](https://openframe.ai) |
| **Authentication Token** | A valid OpenFrame bearer token |
| **Network Access** | Outbound HTTPS to OpenFrame API endpoints |

---

## Environment Variables

The following environment variables may be needed during build and runtime:

| Variable | Purpose | Example |
|----------|---------|---------|
| `OPENSSL_ROOT_DIR` | Path to OpenSSL installation | Set if CMake cannot auto-detect |
| `BOOST_ROOT` | Path to Boost installation | Set if CMake cannot auto-detect |
| `OSQUERY_TOOLCHAIN_SYSROOT` | Cross-compilation sysroot | Optional, for cross-compile targets |

> Inside code blocks, variables like `$OPENSSL_ROOT_DIR` are referenced directly. In prose, always use backtick notation to avoid ambiguity.

---

## Verification Commands

Run these commands to verify your environment is ready before building:

```bash
# Verify CMake version
cmake --version

# Verify compiler
c++ --version

# Verify Git
git --version

# Verify Python 3
python3 --version

# Verify available memory (Linux)
free -h

# Verify available memory (macOS)
sysctl hw.memsize | awk '{print $2/1073741824 " GB"}'

# Verify disk space
df -h .
```

Expected output example:

```text
cmake version 3.28.1
g++ (Ubuntu 12.3.0) 12.3.0
git version 2.43.0
Python 3.11.6
              total        used        free
Mem:            31G        4.2G         27G
Disk: 95G available
```

---

## Build Tools Compatibility Matrix

| CMake Version | Clang Version | GCC Version | Status |
|--------------|--------------|------------|--------|
| 3.21+ | 12+ | 10+ | ✅ Supported |
| 3.18-3.20 | 11 | 9 | ⚠️ May work, not tested |
| < 3.18 | < 11 | < 9 | ❌ Not supported |

---

## Community Support

If you run into dependency issues, join the community:

- **Slack:** [OpenMSP Community](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA)
- **Platform:** [openmsp.ai](https://www.openmsp.ai/)
