#----------------------------------------------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2026 Mark Schofield
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#----------------------------------------------------------------------------------------------------------------------
#
# This CMake toolchain file configures a CMake, non-'Visual Studio Generator' build to use
# the Enterprise Windows Driver Kit.
#
# The toolchain expects the following environment variables to have been initialized by the EWDK build environment:
#
# | Environment Variable                        | Description                                                                                           |
# |---------------------------------------------|-------------------------------------------------------------------------------------------------------|
# | VCToolsVersion                              | The version of the MSVC platform toolset to use (e.g. 14.31.31103).                                   |
# | VSCMD_ARG_HOST_ARCH                         | The architecture of the host tooling to use (e.g. x86).                                               |
# | VSCMD_ARG_TGT_ARCH                          | The architecture to build for (e.g. arm64).                                                           |
# | VSINSTALLDIR                                | The path to the Visual Studio instance to use.                                                        |
# | WindowsSdkDir                               | The path to the Windows SDK directory                                                                 |
# | Version_Number                              | The version of the Windows SDK to use.                                                                |
#
# The following variables can be used to configure the behavior of this toolchain file:
#
# | CMake Variable                              | Description                                                                                           |
# |---------------------------------------------|-------------------------------------------------------------------------------------------------------|
# | CMAKE_SYSTEM_VERSION                        | The version of the operating system for which CMake is to build. Defaults to the host version.        |
# | VS_EXPERIMENTAL_MODULE                      | Whether experimental module support should be enabled.                                                |
# | VS_USE_SPECTRE_MITIGATION_RUNTIME           | Whether the compiler should link with a runtime that uses 'Spectre' mitigations. Defaults to 'OFF'.   |
#
# The toolchain file will set the following variables:
#
# | CMake Variable                              | Description                                                                                           |
# |---------------------------------------------|-------------------------------------------------------------------------------------------------------|
# | CMAKE_C_COMPILER                            | The path to the C compiler to use.                                                                    |
# | CMAKE_CXX_COMPILER                          | The path to the C++ compiler to use.                                                                  |
# | CMAKE_MT                                    | The path to the 'mt.exe' tool to use.                                                                 |
# | CMAKE_RC_COMPILER                           | The path tp the 'rc.exe' tool to use.                                                                 |
# | CMAKE_SYSTEM_NAME                           | "Windows", when cross-compiling                                                                       |
# | CMAKE_SYSTEM_PROCESSOR                      | The architecture to build for (e.g. ARM64).                                                           |
# | CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE | The architecture of the host tooling to use (e.g. x86).                                               |
# | CMAKE_VS_PLATFORM_TOOLSET_VERSION           | The version of the MSVC platform toolset to use (e.g. 14.31.31103).                                   |
# | CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION    | The version of the Windows SDK to use.                                                                |
# | CMAKE_WINDOWS_KITS_10_DIR                   | The path to the Windows SDK directory                                                                 |
# | MSVC                                        | 1                                                                                                     |
# | MSVC_VERSION                                | The '<major><minor>' version of the C++ compiler being used. For example, '1929'                      |
# | VS_INSTALLATION_PATH                        | The path to the Visual Studio instance to use.                                                        |
# | WIN32                                       | 1                                                                                                     |
# | EWDK                                        | 1                                                                                                     |
#
# Resources:
#   <https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html>
#
cmake_minimum_required(VERSION 3.20)

include_guard()

if("$ENV{BuildLabSetupRoot}" STREQUAL "")
    message(FATAL_ERROR "The ${CMAKE_CURRENT_LIST_FILE} toolchain can only be used from an initialized EWDK build prompt.")
endif()

if("$ENV{VSCMD_ARG_TGT_ARCH}" STREQUAL "x64")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
        # Not cross-compiling
    else()
        set(CMAKE_SYSTEM_PROCESSOR "AMD64")
    endif()
elseif($ENV{VSCMD_ARG_TGT_ARCH} STREQUAL "arm64")
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
        # Not cross-compiling
    else()
        set(CMAKE_SYSTEM_PROCESSOR "ARM64")
    endif()
elseif($ENV{VSCMD_ARG_TGT_ARCH} STREQUAL "x86")
    set(CMAKE_SYSTEM_PROCESSOR "X86")
else()
    message(FATAL_ERROR "The 'VSCMD_ARG_TGT_ARCH' environment variable specifies an unsupported target architecture: '$ENV{VSCMD_ARG_TGT_ARCH}'")
endif()

set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE $ENV{VSCMD_ARG_HOST_ARCH})
if(NOT CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
    message(FATAL_ERROR "The 'VSCMD_ARG_HOST_ARCH' environment variable should specify the architecture of the host toolset to use.")
endif()

set(VS_INSTALLATION_PATH $ENV{VSINSTALLDIR})
if(NOT VS_INSTALLATION_PATH)
    message(FATAL_ERROR "The 'VSINSTALLDIR' environment variable should specify the location of Visual Studio.")
endif()

set(VS_PLATFORM_TOOLSET_VERSION $ENV{VCToolsVersion})
if(NOT VS_PLATFORM_TOOLSET_VERSION)
    message(FATAL_ERROR "The 'VCToolsVersion' environment variable should specify the VC toolset version to use.")
endif()

set(CMAKE_WINDOWS_KITS_10_DIR $ENV{WindowsSdkDir})
if(NOT CMAKE_WINDOWS_KITS_10_DIR)
    message(FATAL_ERROR "The 'WindowsSdkDir' environment variable should specify the root of the Windows SDK.")
endif()

set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION $ENV{Version_Number})
if(NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    message(FATAL "The 'Version_Number' environment variable should specify the version of the Windows SDK to use.")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/Windows.MSVC.toolchain.cmake")

set(EWDK 1)
