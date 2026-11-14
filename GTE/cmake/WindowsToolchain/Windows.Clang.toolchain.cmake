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
# the Clang compilers and tools on Windows.
#
# The following variables can be used to configure the behavior of this toolchain file:
#
# | CMake Variable                              | Description                                                                                                              |
# |---------------------------------------------|--------------------------------------------------------------------------------------------------------------------------|
# | CLANG_TIDY_CHECKS                           | List of rules clang-tidy should check. Defaults not set.                                                                 |
# | CMAKE_SYSTEM_PROCESSOR                      | The processor to compiler for. One of 'X86', 'AMD64', 'ARM', 'ARM64'. Defaults to ${CMAKE_HOST_SYSTEM_PROCESSOR}.        |
# | CMAKE_C_COMPILER_FRONTEND_VARIANT           | If set to 'MSVC', configures Clang to use the MSVC-compatible compiler driver for C.                                     |
# | CMAKE_CXX_COMPILER_FRONTEND_VARIANT         | If set to 'MSVC', configures Clang to use the MSVC-compatible compiler driver for C++.                                   |
# | CMAKE_SYSTEM_VERSION                        | The version of the operating system for which CMake is to build. Defaults to '10.0.19041.0'.                             |
# | CMAKE_VS_PRODUCTS                           | One or more Visual Studio Product IDs to consider. Defaults to '*'                                                       |
# | CMAKE_VS_VERSION_PRERELEASE                 | Whether 'prerelease' versions of Visual Studio should be considered. Defaults to 'OFF'                                   |
# | CMAKE_VS_VERSION_RANGE                      | A verson range for VS instances to find. For example, '[16.0,17.0)' will find versions '16.*'. Defaults to '[16.0,17.0)' |
# | CMAKE_WINDOWS_KITS_10_DIR                   | The location of the root of the Windows Kits 10 directory.                                                               |
# | TOOLCHAIN_ADD_VS_NINJA_PATH                 | Whether the toolchain should add the path to the VS Ninja to the CMAKE_SYSTEM_PROGRAM_PATH. Defaults to 'ON'.            |
# | TOOLCHAIN_UPDATE_PROGRAM_PATH               | Whether the toolchain should update CMAKE_PROGRAM_PATH. Defaults to 'ON'.                                                |
# | VS_INSTALLATION_PATH                        | The location of the root of the Visual Studio installation. If not specified VSWhere will be used to search for one.     |
#
# The toolchain file will set the following variables:
#
# | CMake Variable                              | Description                                                                                           |
# |---------------------------------------------|-------------------------------------------------------------------------------------------------------|
# | CMAKE_C_COMPILER                            | The path to the C compiler to use.                                                                    |
# | CMAKE_CXX_CLANG_TIDY                        | The commandline clang-tidy is used if CLANG_TIDY_CHECKS was set.                                      |
# | CMAKE_CXX_COMPILER                          | The path to the C++ compiler to use.                                                                  |
# | CMAKE_MT                                    | The path to the 'mt.exe' tool to use.                                                                 |
# | CMAKE_RC_COMPILER                           | The path tp the 'rc.exe' tool to use.                                                                 |
# | CMAKE_SYSTEM_NAME                           | "Windows", when cross-compiling                                                                       |
# | VS_INSTALLATION_PATH                        | The location of the root of the Visual Studio installation.                                           |
# | WIN32                                       | 1                                                                                                     |
#
# Resources:
#   <https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html>
#
cmake_minimum_required(VERSION 3.20)

include_guard()

option(TOOLCHAIN_UPDATE_PROGRAM_PATH "Whether the toolchain should update CMAKE_PROGRAM_PATH." ON)
option(TOOLCHAIN_ADD_VS_NINJA_PATH "Whether the toolchain should add the path to the VS Ninja to the CMAKE_SYSTEM_PROGRAM_PATH." ON)

set(UNUSED ${CMAKE_TOOLCHAIN_FILE}) # Note: only to prevent cmake unused variable warninig
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
    CMAKE_C_COMPILER
    CMAKE_CXX_COMPILER
    CMAKE_MT
    CMAKE_RC_COMPILER
    CMAKE_SYSTEM_PROCESSOR
    CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE
    CMAKE_VS_PRODUCTS
    CMAKE_VS_VERSION_PRERELEASE
    CMAKE_VS_VERSION_RANGE
    CMAKE_WINDOWS_KITS_10_DIR
    VS_INSTALLATION_PATH
    VS_INSTALLATION_VERSION
    VS_PLATFORM_TOOLSET_VERSION
)
set(WIN32 1)

# If `CMAKE_SYSTEM_PROCESSOR` isn't set, default to `CMAKE_HOST_SYSTEM_PROCESSOR`
if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_HOST_SYSTEM_PROCESSOR})
endif()

# If `CMAKE_SYSTEM_PROCESSOR` is not equal to `CMAKE_HOST_SYSTEM_PROCESSOR`, this is cross-compilation.
# CMake expects `CMAKE_SYSTEM_NAME` to be set to reflect cross-compilation.
if(NOT (CMAKE_SYSTEM_PROCESSOR STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR}))
    set(CMAKE_SYSTEM_NAME Windows)
endif()

if(NOT CMAKE_VS_PRODUCTS)
    set(CMAKE_VS_PRODUCTS "*")
endif()

if(NOT CMAKE_VS_VERSION_PRERELEASE)
    set(CMAKE_VS_VERSION_PRERELEASE OFF)
endif()

if(NOT CMAKE_VS_VERSION_RANGE)
    set(CMAKE_VS_VERSION_RANGE "[16.0,)")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/VSWhere.cmake")

if(NOT CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE)
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL ARM64)
        set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE arm64)
    else()
        set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE x64)
    endif()
endif()

# Find Clang
#
# Search for a Visual Studio installation that includes the Clang toolset, if one isn't found, look for any Visual
# Studio installation so that other Visual Studio components can be found.
if(NOT VS_INSTALLATION_PATH)
    findVisualStudio(
        VERSION ${CMAKE_VS_VERSION_RANGE}
        PRERELEASE ${CMAKE_VS_VERSION_PRERELEASE}
        PRODUCTS ${CMAKE_VS_PRODUCTS}
        REQUIRES
            Microsoft.VisualStudio.Component.VC.Llvm.Clang
        PROPERTIES
            installationVersion VS_INSTALLATION_VERSION
            installationPath VS_INSTALLATION_PATH
    )

    if(NOT VS_INSTALLATION_PATH)
        findVisualStudio(
            VERSION ${CMAKE_VS_VERSION_RANGE}
            PRERELEASE ${CMAKE_VS_VERSION_PRERELEASE}
            PRODUCTS ${CMAKE_VS_PRODUCTS}
            PROPERTIES
                installationVersion VS_INSTALLATION_VERSION
                installationPath VS_INSTALLATION_PATH
        )
    endif()

    if((CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux") AND (EXISTS "/usr/bin/wslpath"))
        # Path properties returned by VSWhere are Windows-style paths. Convert to WSL-style paths on WSL.
        toolchain_to_wsl_path("${VS_INSTALLATION_PATH}" VS_INSTALLATION_PATH)
    endif()
endif()

message(VERBOSE "VS_INSTALLATION_VERSION = ${VS_INSTALLATION_VERSION}")
message(VERBOSE "VS_INSTALLATION_PATH = ${VS_INSTALLATION_PATH}")

if(NOT VS_INSTALLATION_PATH)
    message(FATAL_ERROR "Unable to find Visual Studio")
endif()

cmake_path(NORMAL_PATH VS_INSTALLATION_PATH)

set(VS_MSVC_PATH "${VS_INSTALLATION_PATH}/VC/Tools/MSVC")

# Use 'VS_PLATFORM_TOOLSET_VERSION' to resolve 'CMAKE_VS_PLATFORM_TOOLSET_VERSION'
#
if(NOT VS_PLATFORM_TOOLSET_VERSION)
    file(GLOB VS_PLATFORM_TOOLSET_VERSIONS RELATIVE ${VS_MSVC_PATH} ${VS_MSVC_PATH}/*)
    list(SORT VS_PLATFORM_TOOLSET_VERSIONS COMPARE NATURAL ORDER DESCENDING)
    list(POP_FRONT VS_PLATFORM_TOOLSET_VERSIONS VS_PLATFORM_TOOLSET_VERSION)
    unset(VS_PLATFORM_TOOLSET_VERSIONS)
endif()

set(CMAKE_VS_PLATFORM_TOOLSET_VERSION ${VS_PLATFORM_TOOLSET_VERSION})
set(VS_TOOLSET_PATH "${VS_INSTALLATION_PATH}/VC/Tools/MSVC/${CMAKE_VS_PLATFORM_TOOLSET_VERSION}")

# Set the tooling variables, include_directories and link_directories
#

# Map CMAKE_SYSTEM_PROCESSOR values to CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE that identifies the tools that should
# be used to produce code for the CMAKE_SYSTEM_PROCESSOR.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
        set(CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE x64)
    elseif((CMAKE_SYSTEM_PROCESSOR STREQUAL ARM)
        OR (CMAKE_SYSTEM_PROCESSOR STREQUAL ARM64)
        OR (CMAKE_SYSTEM_PROCESSOR STREQUAL X86))
        set(CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
    endif()
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
        set(CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE ARM64)
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
        set(CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE x64)
    endif()
endif()

if(NOT CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE)
    message(FATAL_ERROR "Unable identify compiler architecture for CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Map CMAKE_SYSTEM_PROCESSOR values to CMAKE_<LANG>_COMPILER_TARGET values that identifies the target architecture
# for the compiler.
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL X86)
        set(TOOLCHAIN_TARGET "i686-pc-windows-msvc")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
        set(TOOLCHAIN_TARGET "x86_64-pc-windows-msvc")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL ARM)
        set(TOOLCHAIN_TARGET "armv7a-pc-windows-msvc")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL ARM64)
        set(TOOLCHAIN_TARGET "aarch64-pc-windows-msvc")
    endif()
else()
    set(TOOLCHAIN_TARGET "${CMAKE_SYSTEM_PROCESSOR}-pc-windows-msvc")
endif()

if(NOT TOOLCHAIN_TARGET)
    message(FATAL_ERROR "Unable identify compiler architecture for CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR}")
endif()

foreach(LANG C CXX)
    set(CMAKE_${LANG}_COMPILER_TARGET ${TOOLCHAIN_TARGET})
endforeach()
unset(TOOLCHAIN_TARGET)

# Look for clang/clang-cl.
#
# The Visual Studio-distributed clang tools are preferred, followed by the default installation location.
set(TOOLCHAIN_CLANG_HINTS_PATHS "$ENV{ProgramFiles}/LLVM/bin")
list(PREPEND TOOLCHAIN_CLANG_HINTS_PATHS "${VS_INSTALLATION_PATH}/VC/Tools/Llvm/x64/bin")
if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL ARM64)
    list(PREPEND TOOLCHAIN_CLANG_HINTS_PATHS "${VS_INSTALLATION_PATH}/VC/Tools/Llvm/arm64/bin")
endif()

if(NOT CMAKE_C_COMPILER)
    set(TOOLCHAIN_C_COMPILER_EXE clang.exe)
    if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL MSVC)
        set(TOOLCHAIN_C_COMPILER_EXE clang-cl.exe)
    endif()

    find_program(CMAKE_C_COMPILER
        ${TOOLCHAIN_C_COMPILER_EXE}
        HINTS
            ${TOOLCHAIN_CLANG_HINTS_PATHS}
        REQUIRED
    )
endif()

if(NOT CMAKE_CXX_COMPILER)
    set(TOOLCHAIN_CXX_COMPILER_EXE clang++.exe)
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL MSVC)
        set(TOOLCHAIN_CXX_COMPILER_EXE clang-cl.exe)
    endif()

    find_program(CMAKE_CXX_COMPILER
        ${TOOLCHAIN_CXX_COMPILER_EXE}
        HINTS
            ${TOOLCHAIN_CLANG_HINTS_PATHS}
        REQUIRED
    )
endif()
unset(TOOLCHAIN_CLANG_HINTS_PATHS)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL ARM)
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} /EHsc")
endif()

# Compiler
foreach(LANG C CXX RC)
    list(APPEND CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES "${VS_TOOLSET_PATH}/ATLMFC/include")
    list(APPEND CMAKE_${LANG}_STANDARD_INCLUDE_DIRECTORIES "${VS_TOOLSET_PATH}/include")
endforeach()

foreach(LANG C CXX)
    if(CMAKE_${LANG}_COMPILER_FRONTEND_VARIANT STREQUAL MSVC)
        # Add '/X': Do not add %INCLUDE% to include search path
        set(CMAKE_${LANG}_FLAGS_INIT "${CMAKE_${LANG}_FLAGS_INIT} /X")
    endif()
endforeach()

if(VS_USE_SPECTRE_MITIGATION_ATLMFC_RUNTIME)
    # Ensure that the necessary folder and files are present before adding the 'link_directories'
    toolchain_validate_vs_files(
        DESCRIPTION "ATLMFC Spectre libraries"
        FOLDER "${VS_TOOLSET_PATH}/ATLMFC/lib/spectre/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}"
        FILES
            atls.lib
    )
    link_directories("${VS_TOOLSET_PATH}/ATLMFC/lib/spectre/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}")
else()
    link_directories("${VS_TOOLSET_PATH}/ATLMFC/lib/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}")
endif()

if(VS_USE_SPECTRE_MITIGATION_RUNTIME)
    # Ensure that the necessary folder and files are present before adding the 'link_directories'
    toolchain_validate_vs_files(
        DESCRIPTION "Spectre libraries"
        FOLDER "${VS_TOOLSET_PATH}/lib/spectre/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}"
        FILES
            msvcrt.lib vcruntime.lib vcruntimed.lib
    )
    link_directories("${VS_TOOLSET_PATH}/lib/spectre/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}")
else()
    link_directories("${VS_TOOLSET_PATH}/lib/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}")
endif()

link_directories("${VS_TOOLSET_PATH}/lib/x86/store/references")

if(CLANG_TIDY_CHECKS)
    get_filename_component(CLANG_PATH ${CMAKE_CXX_COMPILER} DIRECTORY CACHE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_PATH}/clang-tidy.exe;-checks=${CLANG_TIDY_CHECKS}")
endif()

# Windows Kits
include("${CMAKE_CURRENT_LIST_DIR}/Windows.Kits.cmake")

# If 'TOOLCHAIN_UPDATE_PROGRAM_PATH' is selected, update CMAKE_PROGRAM_PATH.
#
if(TOOLCHAIN_UPDATE_PROGRAM_PATH)
    list(APPEND CMAKE_PROGRAM_PATH "${VS_TOOLSET_PATH}/bin/Host${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}/${CMAKE_VS_PLATFORM_TOOLSET_ARCHITECTURE}")
    list(APPEND CMAKE_PROGRAM_PATH "${WINDOWS_KITS_BIN_PATH}/${CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE}")
endif()

# If the CMAKE_GENERATOR is Ninja-based, and the path to the Visual Studio-installed Ninja is present, add it to
# the CMAKE_SYSTEM_PROGRAM_PATH. 'find_program' searches CMAKE_SYSTEM_PROGRAM_PATH after the environment path, so
# an installed Ninja would be preferred.
#
if( (CMAKE_GENERATOR MATCHES "^Ninja") AND
    (EXISTS "${VS_INSTALLATION_PATH}/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja") AND
    (TOOLCHAIN_ADD_VS_NINJA_PATH))
    list(APPEND CMAKE_SYSTEM_PROGRAM_PATH "${VS_INSTALLATION_PATH}/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja")
endif()
