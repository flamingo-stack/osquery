# Prerequisites

Before building or running this OpenFrame-enhanced distribution of osquery, ensure your system and environment meet the following requirements.

---

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **RAM** | 24 GB | 32 GB |
| **CPU Cores** | 6 cores | 12 cores |
| **Disk Space** | 50 GB | 100 GB |
| **OS** | Linux, macOS, or Windows | Linux (Ubuntu 20.04+) |

> **Note:** The build system compiles osquery and all vendored libraries from source. Insufficient RAM will cause build failures or extremely slow compilation times.

---

## Supported Operating Systems

| OS | Architectures | Notes |
|----|--------------|-------|
| **Linux** | x86_64, aarch64 | Ubuntu 20.04+, CentOS 8+, Fedora 34+ |
| **macOS** | x86_64, aarch64 | macOS 12 (Monterey) or newer |
| **Windows** | x86_64, aarch64 | Windows 10 / Server 2019 or newer |

---

## Required Build Tools

### All Platforms

| Tool | Version | Purpose |
|------|---------|---------|
| **CMake** | ≥ 3.21 | Build system generator |
| **Python 3** | ≥ 3.6 | Code generation scripts |
| **Git** | ≥ 2.x | Source control |
| **Clang / GCC** | Clang ≥ 13 or GCC ≥ 10 | C++ compiler (C++17 required) |

### Linux

| Tool | Version | Purpose |
|------|---------|---------|
| **Clang** | ≥ 13 | Preferred compiler |
| **lld** | ≥ 13 | LLVM linker (faster builds) |
| **build-essential** | — | Base build tools |
| **libssl-dev** | ≥ 1.1.1 | OpenSSL (required for OpenFrame encryption) |

### macOS

| Tool | Version | Purpose |
|------|---------|---------|
| **Xcode Command Line Tools** | Latest | Compiler toolchain |
| **Homebrew** | Latest | Package manager |

### Windows

| Tool | Version | Purpose |
|------|---------|---------|
| **Visual Studio** | 2019 or 2022 | MSVC compiler |
| **Windows SDK** | ≥ 10.0.18362 | Platform headers |

---

## OpenSSL Requirement

The OpenFrame encryption service (`OpenframeEncryptionService`) requires **OpenSSL** with AES-GCM support. This is used for:

- AES-256-GCM encryption and decryption of authentication tokens
- Base64 encoding/decoding of encrypted payloads

Verify OpenSSL is available:

```bash
openssl version
```

Expected output example:

```text
OpenSSL 3.0.2 15 Mar 2022 (Library: OpenSSL 3.0.2 15 Mar 2022)
```

---

## Node.js / JavaScript Tools (Documentation Pipeline Only)

The `package.json` at the repository root lists tooling dependencies for the documentation generation pipeline — these are **not** required to build or run osquery itself:

| Package | Purpose |
|---------|---------|
| `@voltagent/core` | Documentation agent (pipeline tool) |
| `@ai-sdk/anthropic` | AI SDK (pipeline tool) |
| `@anthropic-ai/sdk` | Anthropic SDK (pipeline tool) |
| `glob` | File globbing (pipeline tool) |
| `zod` | Schema validation (pipeline tool) |

> **Important:** These packages support documentation generation tooling only. They are not runtime or build dependencies for osquery.

---

## Verification Checklist

Run these commands to verify your environment before starting:

```bash
# Check CMake version (need >= 3.21)
cmake --version

# Check Clang version (need >= 13)
clang --version

# Check Python version (need >= 3.6)
python3 --version

# Check Git
git --version

# Check OpenSSL (required for OpenFrame encryption)
openssl version

# Check disk space (need >= 50 GB free)
df -h .

# Check available memory (need >= 24 GB)
free -h
```

---

## Account and Access Requirements

| Resource | Required For | Notes |
|----------|-------------|-------|
| **OpenFrame account** | OpenFrame platform integration | Sign up at [openframe.ai](https://openframe.ai) |
| **Flamingo account** | MSP platform features | Sign up at [flamingo.run](https://flamingo.run) |
| **OpenMSP Slack** | Community support | [Join here](https://join.slack.com/t/openmsp/shared_invite/zt-36bl7mx0h-3~U2nFH6nqHqoTPXMaHEHA) |

---

## Environment Variables

The following environment variables may be required depending on your build and deployment configuration:

| Variable | Purpose | Required |
|----------|---------|----------|
| `OPENFRAME_TOKEN_PATH` | Path to the encrypted OpenFrame token file | For OpenFrame integration |
| `OPENFRAME_SECRET_KEY` | 256-bit secret key for AES-256-GCM token decryption | For OpenFrame integration |

> **Security Note:** Never hardcode secret keys in shell scripts or configuration files. Use a secrets manager or secure environment injection.

---

## Next Steps

Once your environment is verified:

- Follow the [Quick Start](quick-start.md) to build and run osquery in minutes.
- Complete [First Steps](first-steps.md) to configure your first query packs.
