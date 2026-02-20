# DevOps Summary/Overview

## Introduction

This document summarizes the design decisions made during the discussions on the topic of defining the openDAQ DevOps structure. We use the term DevOps, for the lack of a better word, to combine continuous integration infrastructure and module development processes.

## Continuous integration

Continuous integration (CI) in openDAQ corresponds to a set of automated pipelines that take care of building, testing, packaging and deploying openDAQ.

The CI infrastructure is built on top of GitHub Actions. In GitHub Actions, the role of a pipeline is played by a **workflow** -- a YAML file in `.github/workflows/` that defines a set of **job**s triggered by an event (push, pull request, schedule, completion of another workflow in the same repository, etc). 

A workflow contains one or more **job**s. Each job runs on a separate virtual machine, called a **runner**, and either consists of one or more **step**s or delegates to another workflow, called a **reusable workflow**.

Each step is a shell process that executes a specific set of commands. Steps run sequentially on the same runner instance.

When a job delegates to a reusable workflow, the calling job serves only as a reference point. The reusable workflow's jobs run on their own separate runners. The calling workflow can pass inputs and secrets to the reusable workflow, and receive data back through workflow outputs and **artifact**s.

To avoid duplicating the same steps across multiple jobs, GitHub provides **action**s -- reusable units of CI logic defined separately and called as a single step. An action is either a YAML file containing one or more steps (a **composite action**) or a JavaScript/TypeScript script (a **JavaScript action**). Actions are defined in `.github/actions/` within the repository or published to the GitHub Marketplace.

A job can be parameterized using a **matrix** -- a set of variable combinations (e.g. OS, compiler, build type) that GitHub expands into multiple parallel job instances, one per combination.

Every time a workflow is triggered, GitHub assigns a unique numeric identifier called a **run ID**. All step logs (stdout/stderr) and artifacts produced during the run are associated with this ID. The progress and results of a workflow run can be monitored in the GitHub UI at:

```
https://github.com/{owner}/{repo}/actions/runs/{run-id}
```

GitHub provides the ability to save build results (compiled binaries, test reports, packages, etc.) as artifacts. Artifacts are tied to the run ID, can be up to 5 GB each, and have a configurable retention period (1--90 days, default 90). Within the same workflow, artifacts are accessible to other jobs. Other workflows can also download these artifacts if they know the run ID.

## Package naming convention

A build target is commonly identified by a **triplet** -- a structured string that encodes the platform a binary was built for. There is no single universal standard for triplets. The most widely adopted is the GNU target triplet format:

```
<arch>-<vendor>-<os>-<abi>
```

However, GNU's architecture values (e.g. `i386`, `i586`, `i686`, etc) describe specific x86 instruction set generations and are largely obsolete today.

The Conan package manager addresses the same problem through its `settings` system, with a clean set of aliases for architecture, operating system, compiler, and build type.

The openDAQ package naming convention follows the GNU triplet structure but uses Conan values for individual components:

```
opendaq-<version>[-<short-sha>]-<arch>-<os>-<compiler>-<compiler-version>-<build-type>
```

The `<vendor>` field is omitted as irrelevant. The `<abi>` field is replaced by `<compiler>-<compiler-version>`, which provides more actionable information for binary compatibility.

**Components:**

- `<arch>` -- `x86`, `x86_64`, `armv7`, `armv8`
- `<os>` -- `linux`, `windows`, `macos`
- `<compiler>` -- `gcc`, `clang`, `apple-clang`, `msvc`, `intel-cc`
- `<compiler-version>` -- major toolset version
- `<build-type>` -- `release`, `debug`

> **Note `<compiler-version>`:** 
- `msvc` is the toolset version (CMake's `MSVC_TOOLSET_VERSION`, e.g. `143`). 
- `gcc`/`clang` is the major version of the compiler (e.g. `14`, `16`). 
- `intel-cc` is year-based (e.g. `2025`).

**Versioning:** 

The version is read from the `opendaq_version` file and follows the format `X.YY.Z`, optionally suffixed with `dev`. When the `dev` suffix is present, it is replaced by the short git SHA in the package name; otherwise, the version is used as-is. The `main` branch always carries the `dev` suffix, while `release/*` branches typically do not.

## openDAQ pipelines

The openDAQ CI consists of several core workflows and a set of composite actions that encapsulate shared logic between jobs.

**Core workflows:**

- `ci.yml` -- builds SDK sources and runs unit tests 
- `package.yml` -- build distributable packages and bindings (C# .NET and Python Wheels)
- `deploy.yml` -- deploy distributable packages to AWS S3 buckets
- `trigger_downstream_deployment.yml` -- publishes a User Guide documentation to opendaq.github.io
- `test_external_modules.yml` -- notifies external module repositories of changes in the openDAQ codebase (e.g. the modules can run their own compatibility checks)

**Composite actions:**

- `build-sdk` -- installs dependencies, compiles sources, optionally runs tests and creates packages
- `build-doc` -- builds Doxygen API reference documentation (requires `build-sdk` to run first)
- `package-bindings-python` -- builds Python wheels for multiple Python versions (requires `build-sdk` to run first)
- `package-bindings-csharp` -- builds C# bindings and generates artifacts for NuGet packaging (requires `build-sdk` to run first)
- `package-version-detect` -- detects OS, architecture, git SHA, and package version (requires `actions/checkout`)
- `setup-compiler-icx` -- installs and configures the Intel oneAPI C++ Compiler (ICX)
- `source-script` -- utility that sources a setup script and makes its variables available to subsequent steps

> **Note:** Since each step runs in its own shell process, environment variables set in one step are not visible to the next. To share variables across steps, they must be appended to the file pointed to by the `GITHUB_ENV` environment variable in the format `VAR_NAME=value`. GitHub reads this file before each step and injects the variables into the new process environment. The `source-script` action automates this: it captures the environment before and after sourcing a script, compares the two snapshots, and writes any new or changed variables to the `${GITHUB_ENV}` file.

### ci.yml

The primary CI workflow. Compiles SDK sources across multiple platforms and compilers, runs unit tests, and uploads test results (gtest XML) as artifacts.

**Triggers:** pull request (non-draft), `workflow_dispatch`, `workflow_call`

> **Note:** The `workflow_call` trigger is used by `unstable_tests.yml` (nightly schedule) to run experimental tests with a dedicated preset.

**Build and test matrix:**

| Status | OS | Architecture | Compiler | Compiler version | Build Type | Notes |
|--------|-----|--------------|----------|------------------|------------|-------|
| 🟢 | windows | x86_64 | msvc | default | release | Windows VS 2022 x64 Release |
| 🟢 | windows | x86_64 | msvc | default | debug | Windows VS 2022 x64 Debug |
| 🟢 | windows | x86 | msvc | default | release | Windows VS 2022 Win32 Release |
| 🟡 | windows | x86 | msvc | default | debug | |
| 🟢 | windows | x86_64 | clang | default | release | Windows Clang Release |
| 🟡 | windows | x86_64 | clang | default | debug | |
| 🔴 | windows | x86_64 | clang | oldest/latest | release/debug | |
| 🟡 | windows | x86 | clang | default | release | |
| 🟡 | windows | x86 | clang | default | debug | |
| 🔴 | windows | x86 | clang | oldest/latest | release/debug | |
| 🟢 | windows | x86_64 | gcc | default | release | Windows GCC Release (tests disabled) |
| 🟡 | windows | x86_64 | gcc | default | debug | |
| 🔴 | windows | x86_64 | gcc | oldest/latest | release/debug | |
| 🟡 | windows | x86 | gcc | default | release | |
| 🟡 | windows | x86 | gcc | default | debug | |
| 🔴 | windows | x86 | gcc | oldest/latest | release/debug | |
| 🟢 | windows | x86_64 | intel-cc | 2025 | release | Windows Intel-LLVM Release |
| 🟡 | windows | x86_64 | intel-cc | 2025 | debug | |
| 🟡 | windows | x86_64 | intel-cc | oldest/latest | release/debug | |
| 🟢 | linux | x86_64 | clang | 14 | release | Ubuntu Latest clang-14 Release |
| 🟡 | linux | x86_64 | clang | 14 | debug | disabled (test_py_opendaq -- failed) |
| 🟢 | linux | x86_64 | clang | 16 | release | Ubuntu Latest clang-16 Release |
| 🟡 | linux | x86_64 | clang | 16 | debug | disabled (test_py_opendaq -- failed) |
| 🟡 | linux | x86 | clang | oldest/latest | release/debug | |
| 🟢 | linux | x86_64 | gcc | 9 | release | Ubuntu Latest gcc-9 Release |
| 🟡 | linux | x86_64 | gcc | 9 | debug | disabled (test_py_opendaq -- failed) |
| 🟢 | linux | x86_64 | gcc | 14 | release | Ubuntu Latest gcc-14 Release |
| 🟡 | linux | x86_64 | gcc | 14 | debug | disabled (test_py_opendaq -- failed) |
| 🟢 | linux | x86 | gcc | default | release | Ubuntu Latest gcc x86_32 Release |
| 🟡 | linux | x86 | gcc | default | debug | |
| 🔴 | linux | x86_64 | intel-cc | oldest/latest | release/debug | |
| 🟡 | linux | armv8 | gcc | oldest/latest | release/debug | |
| 🔴 | linux | armv7 | clang | oldest | debug | no armv7 runner |
| 🟢 | macos | x86_64 | apple-clang | default | release | MacOS 15 Intel Clang Release |
| 🟡 | macos | x86_64 | apple-clang | default | debug | |
| 🟢 | macos | armv8 | apple-clang | default | release | MacOS Latest Clang Release |
| 🟡 | macos | armv8 | apple-clang | default | debug | |

**Statuses:**

- 🟢 implemented
- 🟡 can be added
- 🔴 needs development

**Compiler version:**

- `default` -- version provided by the runner or package manager
- `9`, `14`, `16`, `143`, `2025`, etc -- explicitly pinned specific versions
- `oldest` / `latest` -- testing against the oldest and latest available versions on the runner

### package.yml

Builds distributable packages: native installers (NSIS, DEB), Python wheels, .NET NuGet packages, Doxygen API reference, and the simulator VM image.

**Triggers:** push to `main` or `release/**`, `workflow_dispatch`

**Installer matrix:**

| Status | OS | Architecture | Compiler | Compiler version | Build Type | Notes |
|--------|-----|--------------|----------|------------------|------------|-------|
| 🟢 | windows | x86_64 | msvc | default | release | Windows VS 2022 x64 Release |
| 🟢 | windows | x86 | msvc | default | release | Windows VS 2022 Win32 Release |
| 🟡 | windows | x86_64 | gcc | default | release | |
| 🟡 | windows | x86 | gcc | default | release | |
| 🟡 | windows | x86_64 | clang | default | release | |
| 🟡 | windows | x86 | clang | default | release | |
| 🔴 | windows | x86_64 | intel-cc | default | release | |
| 🟢 | linux | x86_64 | gcc | default | release | Linux x86_64 GCC Release |
| 🟡 | linux | x86 | gcc | default | release | |
| 🟡 | linux | x86_64 | clang | default | release | |
| 🟡 | linux | x86 | clang | default | release | |
| 🟢 | linux | armv8 | gcc | default | release | Linux ARM64 GCC Release |
| 🔴 | linux | armv7 | gcc | default | release | |
| 🔴 | linux | armv8 | clang | default | release | |
| 🔴 | linux | armv7 | clang | default | release | |
| 🟡 | macos | armv8 | apple-clang | default | release | |
| 🟡 | macos | x86_64 | apple-clang | default | release | |

> **TBD:** Package both release and debug binaries (or release with debug information).

**Python wheels:**

| Status | OS | Architecture | Notes |
|--------|-----|-------------|-------|
| 🟢 | windows | x86_64 | |
| 🟢 | manylinux | x86_64 | manylinux_2_28 gcc Release |
| 🔴 | manylinux | armv8 | quay.io/pypa/manylinux_2_28_aarch64 |
| 🟢 | macos | x86_64 | macOS x86_64 Clang Release |
| 🟢 | macos | armv8 | macOS ARM64 Clang Release |

Python versions: 3.8, 3.9, 3.10, 3.11, 3.12, 3.13, 3.14

**.NET NuGet:**

During the Windows package build, C# bindings are compiled and a NuGet package is created. A separate job then repacks it together with Linux binaries using the reusable workflow `reusable_nuget_creation_and_test.yml`.

| Status | OS | Architecture | Compiler | Build Type | Notes |
|--------|-----|-------------|----------|------------|-------|
| 🟢 | windows | x86_64 | msvc | release | produces NuGet package with Windows DLLs |
| 🟢 | linux | x86_64 | gcc | release | produces .so binaries for adding to NuGet |

**Simulator:**

Builds a VirtualBox VM image (OVA) that runs an openDAQ server application with the reference device simulator. The simulator application is compiled during the Linux x86_64 GCC package build with `-DDAQSIMULATOR_ENABLE_SIMULATOR_APP=ON`. A separate job then packages it into an Ubuntu VM using Vagrant and VirtualBox, and exports the result as an OVA appliance.

### deploy.yml

Deploys artifacts produced by `package.yml` to AWS S3 buckets.

**Triggers:** `workflow_run` (on completion of `package.yml` on the `main` branch), `workflow_dispatch`

The workflow requires the run ID of a `package.yml` run in order to download its artifacts and deploy them to an AWS S3 bucket. The first job (`resolve`) determines three values: run ID, branch and commit hash:

- `workflow_run` -- run ID, branch and commit hash are taken directly from the event context (`github.event.workflow_run.id`, `.head_branch`, `.head_sha`).
- `workflow_dispatch` -- the user provides the `run_id` parameter. The branch and commit hash are then resolved via the GitHub API (`gh api repos/{owner}/{repo}/actions/runs/{run_id}`).

The commit hash is used to check out the sources where necessary (e.g. to determine the package version). The branch name determines the target directory in the S3 bucket: artifacts from `main` are deployed to the development directory, while artifacts from `release/*` branches are deployed to the release directory. The directory names in the bucket match the branch names.

### Documentation

The openDAQ documentation consists of two parts: **API Reference** and **User Guide**.

**API Reference** is generated by Doxygen during the Linux x86_64 GCC package build (in `package.yml`) and uploaded as an artifact.

**User Guide** is built with Antora as a separate job (`prepare_doc`) within `deploy.yml`. The guide is deployed to the documentation website and archived to S3. The target site depends on the branch:

- `main` -- deployed to docs-dev.opendaq.com
- `release/*` -- deployed to docs.opendaq.com

A separate workflow (`trigger_downstream_deployment.yml`) publishes the User Guide to opendaq.github.io.

> **TBD:** Configure docs.opendaq.com / docs-dev.opendaq.com to redirect to opendaq.github.io.

### External module testing

The `test_external_modules.yml` workflow triggers the build and test pipelines of officially supported openDAQ modules and reports the results. Downstream repositories should use a common openDAQ workflow.

TBD