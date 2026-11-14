# CMake Toolchain files for Windows

A CMake toolchain file describes the set of tools and utilities for compiling code in CMake. This repo provides
toolchains that describes how to compile using MSVC and Clang in CMake, with the goal of making Windows CMake builds more
canonical to reduce the 'barrier-to-entry' to build code for Windows.

This repo contains three toolchains:

1. [Windows.MSVC.toolchain.cmake](./Windows.MSVC.toolchain.cmake) - Uses the MSVC compiler from an installed Visual Studio
instance.

2. [Windows.Clang.toolchain.cmake](./Windows.Clang.toolchain.cmake) - Uses Clang from either an LLVM installation or from
a Visual Studio installation.

3. [Windows.EWDK.toolchain.cmake](./Windows.EWDK.toolchain.cmake) - Uses the MSVC compiler from an [Enterprise Windows Driver Kit][ewdk].
This toolchain requires that CMake be invoked from an EWDK build prompt.

Each toolchain will find a compiler, runtime and a Windows SDK.

[![build status](https://github.com/MarkSchofield/Toolchain/actions/workflows/ci.yaml/badge.svg?branch=main)](https://github.com/MarkSchofield/Toolchain/actions/workflows/ci.yaml?query=branch%3Amain)

The CMake scripts for Windows-specific build tasks that used to be in this repository can now be found in [MarkSchofield/WindowsCMake](https://github.com/MarkSchofield/WindowsCMake).

## But I can build with MSVC in CMake already...?

Yes, but you're probably either:

  1. using a "Visual Studio Generator". In which case, CMake will emit - and then build - a Visual Studio Solution file
     that knows how to find the build tools and utilities.

  2. running from a Visual Studio "Command Prompt". In which case, the command prompt is hard-coded to use a set of
     tools and utilities, which means that you always need to initialize your command prompt before getting started.

By using a Toolchain file to describe the tools and utilities to build in CMake-terms, you can use other generators
(e.g. Ninja) and don't have to initialize an environment to run a build.

## So this is a replacement for Visual Studio!?

No, no. This uses the tools and utilities from installed Visual Studio but from a CMake build.

## So this solves all my build problems!?

No. This is an early attempt at making Visual Studio tooling easier to use from CMake. There's definitely gaps -
there's no support for "Windows Universal" builds, for example. But it gets enough of a stake-in-the-ground to start a
conversation.

## How do I use this?

Simply specify one of the toolchain files to your CMake builds. See [the documentation for CMake toolchains][cmake-toolchains]
for more details on Toolchain files and how to consume them.

Each toolchain file (['Windows.MSVC.toolchain.cmake'](./Windows.MSVC.toolchain.cmake), ['Windows.Clang.toolchain.cmake'](./Windows.Clang.toolchain.cmake),
or [Windows.EWDK.toolchain.cmake](./Windows.EWDK.toolchain.cmake)) has details on the various CMake properties
that can be used to configure the build. And [the example folder](./example) provides a CMake project that builds a
variety of Windows projects, using each of the toolchain files.

## WindowsToolchain and VCPkg

VCPkg - <https://github.com/microsoft/vcpkg> - is a Package Manager for C++. When using WindowsToolchain with VCPkg:

1. The `vcpkg.cmake` script must be specified as the toolchain for CMake builds - as per [the VCPkg documentation](https://github.com/microsoft/vcpkg#getting-started).

2. The path to the WindowsToolchain file - `Windows.MSVC.toolchain.cmake` - should be specified as the `VCPKG_CHAINLOAD_TOOLCHAIN_FILE`. The VCPkg toolchain will load the specified `VCPKG_CHAINLOAD_TOOLCHAIN_FILE`.

Note:

* VCPkg has default functionality to copy runtime dependencies from VCPkg-based packages during a CMake build. The functionality - implemented in [`applocal.ps1`](https://github.com/microsoft/vcpkg/blob/0ba60bfef5dea4cb2599daa7ad8364e309835a68/scripts/buildsystems/msbuild/applocal.ps1) - requires that `dumpbin`, `llvm-objdump` or `objdump` be found by PowerShell's `Get-Command`. On Windows, `applocal.ps1` is typically executed so that it would find `dumpbin` through the `PATH` environment variable, but since WindowsToolchain doesn't configure the `PATH` environment variable `applocal.ps1` fails.

  To disable this behavior, `VCPKG_APPLOCAL_DEPS` to `OFF` in your `CMakePresets.json`, on the CMake configuration command-line, or before the top-level `project()` call. If dependencies need to be copied during a build, use custom commands to copy them. For example, the following CMake snippet will copy dependencies for a target called `CommandLine`, for dependencies that support the `$<TARGET_RUNTIME_DLLS:tgt>` generator expression:

  ```cmake
  if(WIN32)
      add_custom_command(TARGET CommandLine POST_BUILD
          COMMAND "${CMAKE_COMMAND};-E;$<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:CommandLine>>,copy;$<TARGET_RUNTIME_DLLS:CommandLine>;$<TARGET_FILE_DIR:CommandLine>,true>"
          COMMAND_EXPAND_LISTS
      )
  endif()
  ```

  Note: The `$<TARGET_RUNTIME_DLLS:tgt>` generator expression requires CMake 3.21 or higher.

## Linting

WindowsToolchain uses [`cmakelang`][cmakelang] for linting the CMake files in the codebase. The
[.cmake-format.yaml](./.cmake-format.yaml) file describes the formatting style for the codebase. To run the linting
tools:

1. Install [`cmakelang`][cmakelang] following [the installation instuctions](https://cmake-format.readthedocs.io/en/latest/installation.html).
Note: Since WindowsToolchain uses a `.yaml` file for configuration, make sure to install the `cmakelang[YAML]` package.

2. Run [`./analyze.ps1`](./analyze.ps1)

The [Toolchain CI](.\.github\workflows\ci.yaml) GitHub Workflow enforces the linting rules during PR and CI.

[cmake-toolchains]: https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html "CMake Toolchains"
[cmakelang]: https://cmake-format.readthedocs.io/ "cmakelang"
[ewdk]: https://learn.microsoft.com/en-us/windows-hardware/drivers/develop/using-the-enterprise-wdk "Enterprise Windows Driver Kit"
