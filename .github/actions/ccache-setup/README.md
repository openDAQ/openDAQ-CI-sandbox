# ccache-setup

[![Test](https://github.com/openDAQ/openDAQ-CI-sandbox/actions/workflows/test-ccache-setup.yml/badge.svg)](https://github.com/openDAQ/openDAQ-CI-sandbox/actions/workflows/test-ccache-setup.yml)

Cross-platform build cache setup action with automatic caching. Supports ccache and MSBuildCache (experimental).

## Usage

### ccache (default)

```yaml
- uses: openDAQ/openDAQ-CI-sandbox/.github/actions/ccache-setup@actions/ccache-setup
  with:
    cmake-config-args: '-DCMAKE_CXX_COMPILER=g++-12'
    cmake-build-type: Release
```

### MSBuildCache (experimental, Windows only)

```yaml
- uses: openDAQ/openDAQ-CI-sandbox/.github/actions/ccache-setup@actions/ccache-setup
  id: cache
  with:
    use-msbuild-cache: true
    cmake-build-type: Release
    cache-size: '2048'  # MB

- name: Build
  run: cmake --build build -- ${{ steps.cache.outputs.msbuild-args }}
```

## Inputs

| Input | Description | Default |
|-------|-------------|---------|
| `use-msbuild-cache` | Use MSBuildCache instead of ccache (Windows + VS 17.9+) | `false` |
| `cmake-config-args` | CMake args (extracts compiler for cache key) | |
| `cmake-build-type` | Build type (Release/Debug) | |
| `target-arch` | Target architecture for cross-compilation | |
| `cache-key-prefix` | Cache key prefix (e.g., `test`, `opendaq`) | |
| `cache-size` | Cache size (`2G` for ccache, `2048` MB for msbuild-cache) | |
| `download-only` | Only download cache, no auto-save | `false` |

## Outputs

| Output | Description |
|--------|-------------|
| `cache-hit` | `true` if cache was restored |
| `cache-key` | Generated cache key |
| `cache-dir` | Path to cache directory |
| `msbuild-args` | MSBuild arguments (only when `use-msbuild-cache: true`) |

## Cache key format

```
<type>-[prefix-]<distro>-<arch>-<compiler>-<build-type>-<sha>
```

Examples:
- `ccache-ubuntu-24.04-x86-64-g++-12-release-abc1234`
- `msbuild-windows-nt10-x86-64-cl-release-abc1234`

## MSBuildCache notes

- Requires Visual Studio 17.9+
- Experimental/preview feature
- Creates `Directory.Build.props` and `Directory.Build.targets` in workspace root
- Pass `${{ steps.cache.outputs.msbuild-args }}` to cmake build command
