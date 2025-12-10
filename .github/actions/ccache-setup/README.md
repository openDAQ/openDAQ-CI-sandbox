# ccache-setup

[![Test](https://github.com/openDAQ/openDAQ-CI-sandbox/actions/workflows/test-ccache-setup.yml/badge.svg)](https://github.com/openDAQ/openDAQ-CI-sandbox/actions/workflows/test-ccache-setup.yml)

Cross-platform ccache setup action with automatic caching.

## Usage

```yaml
- uses: openDAQ/openDAQ-CI-sandbox/.github/actions/ccache-setup@main
  with:
    cmake-config-args: '-DCMAKE_CXX_COMPILER=g++-12'
    cmake-build-type: Release
```

## Inputs

| Input | Description | Required |
|-------|-------------|----------|
| `cmake-config-args` | CMake args (extracts compiler for cache key) | No |
| `cmake-build-type` | Build type (Release/Debug) | No |
| `target-arch` | Target architecture for cross-compilation | No |
| `cache-key-prefix` | Cache key prefix (e.g., `test`, `opendaq`) | No |

## Outputs

| Output | Description |
|--------|-------------|
| `cache-hit` | `true` if cache was restored |
| `cache-key` | Generated cache key |

## Cache key format

```
ccache-[prefix-]<distro>-<arch>-<compiler>-<build-type>-<sha>
```

Example: `ccache-ubuntu-24.04-x86-64-g-12-release-abc1234`
