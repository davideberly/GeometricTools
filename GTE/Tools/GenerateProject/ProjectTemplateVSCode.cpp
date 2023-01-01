// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.11

#include "ProjectTemplateVSCode.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <fstream>

TemplateVSCode::TemplateVSCode()
{
}

bool TemplateVSCode::Execute(std::string const& projectName, std::string const& appType)
{
    if (appType == "c")
    {
        return CreateDotVSCodeFolderAndFiles()
            && CreateCMakeSample()
            && CreateCMakeVariants()
            && CreateCodeWorkspace(projectName)
            && CreateCMakeLists(projectName, "Console");
    }
    
    if (appType == "w2")
    {
        return CreateDotVSCodeFolderAndFiles()
            && CreateCMakeSample()
            && CreateCMakeVariants()
            && CreateCodeWorkspace(projectName)
            && CreateCMakeLists(projectName, "Window2");
    }
    
    if (appType == "w3")
    {
        return CreateDotVSCodeFolderAndFiles()
            && CreateCMakeSample()
            && CreateCMakeVariants()
            && CreateCodeWorkspace(projectName)
            && CreateCMakeLists(projectName, "Window3");
    }

    return false;
}

bool TemplateVSCode::CreateDotVSCodeFolderAndFiles()
{
    size_t constexpr numBuffer = 4096;
    std::array<char, numBuffer> buffer{};
    sprintf_s(buffer.data(), numBuffer, "mkdir .vscode");
    int result = system(buffer.data());
    if (result != 0)
    {
        return false;
    }

    std::ofstream output(".vscode/launch.json", std::ios::binary);
    if (!output)
    {
        return false;
    }

    std::string target = ReplaceCRLFbyLF(msLaunch, false);
    output.write((char const*)target.c_str(), target.size());
    output.close();

    output.open(".vscode/settings.json", std::ios::binary);
    if (!output)
    {
        return false;
    }

    target = ReplaceCRLFbyLF(msSettings, false);
    output.write((char const*)target.c_str(), target.size());
    output.close();

    return true;
}

bool TemplateVSCode::CreateCMakeSample()
{
    std::ofstream output("CMakeSample.sh", std::ios::binary);
    if (!output)
    {
        return false;
    }

    std::string target = ReplaceCRLFbyLF(msCMakeSample, false);
    output.write((char const*)target.c_str(), target.size());
    output.close();
    return true;
}

bool TemplateVSCode::CreateCMakeVariants()
{
    std::ofstream output("cmake-variants.json", std::ios::binary);
    if (!output)
    {
        return false;
    }

    std::string target = ReplaceCRLFbyLF(msCMakeVariants, true);
    output.write((char const*)target.c_str(), target.size());
    output.close();
    return true;
}

bool TemplateVSCode::CreateCodeWorkspace(std::string const& projectName)
{
    std::ofstream output(projectName + ".code-workspace", std::ios::binary);
    if (!output)
    {
        return false;
    }

    std::string target = ReplaceCRLFbyLF(msCodeWorkspace, false);
    output.write((char const*)target.c_str(), target.size());
    output.close();
    return true;
}

bool TemplateVSCode::CreateCMakeLists(std::string const& projectName,
    std::string const& applicationType)
{
    std::ofstream output("CMakeLists.txt", std::ios::binary);
    if (!output)
    {
        return false;
    }

    std::string target = msCMakeLists;
    target = std::regex_replace(target, msProjectNamePattern, projectName);
    target = std::regex_replace(target, msApplicationTypePattern, applicationType);
    output.write((char const*)target.c_str(), target.size());
    output.close();
    return true;
}

std::string TemplateVSCode::ReplaceCRLFbyLF(std::string const& source, bool isUTF8)
{
    std::string target{};

    if (isUTF8)
    {
        target += '\xEF';
        target += '\xBB';
        target += '\xBF';
    }

    for (size_t i = 0; i < source.size(); ++i)
    {
        if (source[i] == '\n')
        {
            target += '\xA';
        }
        else
        {
            target += source[i];
        }
    }

    return target;
}


std::regex const TemplateVSCode::msProjectNamePattern("_PROJECT_NAME_");
std::regex const TemplateVSCode::msApplicationTypePattern("_APPLICATION_TYPE_");

std::string const TemplateVSCode::msLaunch =
R"raw({
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Launch Debug Static",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/DebugStatic/${workspaceFolderBasename}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "Launch Release Static",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/ReleaseStatic/${workspaceFolderBasename}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "Launch Debug Shared",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/DebugShared/${workspaceFolderBasename}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "Launch Release Shared",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/ReleaseShared/${workspaceFolderBasename}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
   ]
})raw";

std::string const TemplateVSCode::msSettings =
R"raw({
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
})raw";

std::string const TemplateVSCode::msCMakeSample =
R"raw(#!/bin/bash
# usage: ./CMakeSample.sh BUILD_TYPE LIBRARY_TYPE
# where BUILD_TYPE is in {Debug,Release}
# and where LIBRARY_TYPE is in {Static,Shared}

BUILD_TYPE=$1
LIBRARY_TYPE=$2
GCC=gcc
GXX=g++

if [[ ! "${BUILD_TYPE}" = "Debug" && ! "${BUILD_TYPE}" = "Release" ]]; then
    echo "Invalid build type: ${BUILD_TYPE}, must be in {Debug, Release}"
    exit 1
fi

if [[ ! "${LIBRARY_TYPE}" = "Static" && ! "${LIBRARY_TYPE}" = "Shared" ]]; then
    echo "Invalid library type: ${LIBRARY_TYPE}, must be in {Static, Shared}"
    exit 2
fi

if [ "${BUILD_TYPE}" = "Debug" ]; then
    if [ "${LIBRARY_TYPE}" = "Static" ]; then
        cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_RELEASE_LIB:BOOL=FALSE -DBUILD_SHARED_LIB:BOOL=FALSE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    else
        cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_RELEASE_LIB:BOOL=FALSE -DBUILD_SHARED_LIB:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    fi
    cmake --build ./build --config Debug --target all -- -j 10
else
    if [ "${LIBRARY_TYPE}" = "Static" ]; then
        cmake -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_RELEASE_LIB:BOOL=TRUE -DBUILD_SHARED_LIB:BOOL=FALSE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    else
        cmake -DCMAKE_BUILD_TYPE:STRING=Release -DBUILD_RELEASE_LIB:BOOL=TRUE -DBUILD_SHARED_LIB:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=${GCC} -DCMAKE_CXX_COMPILER:FILEPATH=${GXX} -B./build -G "Unix Makefiles"
    fi
    cmake --build ./build --config Release --target all -- -j 10
fi
)raw";

std::string const TemplateVSCode::msCMakeVariants =
R"raw({
  "build_type": {
    "default": "debug",
    "description": "The CMake build type to use",
    "choices": {
      "debug": {
        "short": "Debug",
        "long": "Emit debug information without performing optimizations",
        "buildType": "Debug",
        "settings": {
          "BUILD_RELEASE_LIB": false
        }
      },
      "release": {
        "short": "Release",
        "long": "Enable optimizations, omit debug info",
        "buildType": "Release",
        "settings": {
          "BUILD_RELEASE_LIB": true
        }
      }
    }
  },
  "library_type": {
    "default": "static",
    "description": "Selects the library type to build for",
    "choices": {
      "static": {
        "short": "Static",
        "long": "Builds the static library",
        "settings": {
          "BUILD_SHARED_LIB": false
        }
      },
      "shared": {
        "short": "Shared",
        "long": "Builds the shared library",
        "settings": {
          "BUILD_SHARED_LIB": true
        }
      }
    }
  }
})raw";

std::string const TemplateVSCode::msCodeWorkspace =
R"raw({
	"folders": [
		{
			"path": "."
		}
	],
	"settings": {}
})raw";

std::string const TemplateVSCode::msCMakeLists =
R"raw(if(COMMAND cmake_policy)
    # Allow VERSION in the project() statement.
    cmake_policy(SET CMP0048 NEW)
endif()

project(_PROJECT_NAME_)

cmake_minimum_required(VERSION 3.8)
option(BUILD_RELEASE_LIB "Build release library" OFF)
option(BUILD_SHARED_LIB "Build shared library" OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_definitions(-DGTE_USE_LINUX -DGTE_USE_ROW_MAJOR -DGTE_USE_MAT_VEC -DGTE_USE_OPENGL -DGTE_DISABLE_PCH)
add_compile_options(-c -Wall -Werror)
if(BUILD_RELEASE_LIB)
    add_compile_definitions(NDEBUG)
    add_compile_options(-O3)
else()
    add_compile_definitions(_DEBUG)
    add_compile_options(-g)
endif()

set(GTE_ROOT ${PROJECT_SOURCE_DIR}/../../..)
set(GTE_INC_DIR ${GTE_ROOT})
set(GTE_LIB_PREFIX ${GTE_ROOT}/lib/${CMAKE_BUILD_TYPE})
set(GTE_EXE_PREFIX ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE})
if(BUILD_SHARED_LIB)
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Shared)
    set(GTE_EXE_DIR ${GTE_EXE_PREFIX}Shared)
else()
    set(GTE_LIB_DIR ${GTE_LIB_PREFIX}Static)
    set(GTE_EXE_DIR ${GTE_EXE_PREFIX}Static)
endif()
set(EXECUTABLE_OUTPUT_PATH ${GTE_EXE_DIR} CACHE PATH "Executable directory" FORCE)
SET(EXECUTABLE_OUTPUT_PATH ${GTE_EXE_DIR})

include_directories(${GTE_INC_DIR})

add_executable(${PROJECT_NAME}
${PROJECT_NAME}Main.cpp
${PROJECT_NAME}_APPLICATION_TYPE_.cpp)

find_package(PNG REQUIRED)
find_package(Threads REQUIRED)
target_link_directories(${PROJECT_NAME} PUBLIC ${GTE_LIB_DIR})
target_link_libraries(${PROJECT_NAME}
gtapplications
gtmathematicsgpu
gtgraphics
GL
EGL
X11
PNG::PNG
Threads::Threads))raw";
