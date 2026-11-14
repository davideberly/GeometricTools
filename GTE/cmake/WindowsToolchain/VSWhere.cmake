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
include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/WSL.cmake")

#[[====================================================================================================================
    toolchain_validate_vs_files
    ---------------------------

    Note: Not supported for consumption outside of the toolchain files.

    Validates the the specified folder exists and contains the specified files.

        toolchain_validate_vs_files(
            <DESCRIPTION <description>>
            <FOLDER <folder>>
            <FILES <file>...>
        )

    If the folder or files are missing, then a FATAL_ERROR is reported.
====================================================================================================================]]#
function(toolchain_validate_vs_files)
    set(OPTIONS)
    set(ONE_VALUE_KEYWORDS FOLDER DESCRIPTION)
    set(MULTI_VALUE_KEYWORDS FILES)

    cmake_parse_arguments(PARSE_ARGV 0 VS "${OPTIONS}" "${ONE_VALUE_KEYWORDS}" "${MULTI_VALUE_KEYWORDS}")

    if(NOT EXISTS ${VS_FOLDER})
        message(FATAL_ERROR "Folder not present - ${VS_FOLDER} - ensure that the ${VS_DESCRIPTION} are installed with Visual Studio.")
    endif()

    foreach(FILE ${VS_FILES})
        if(NOT EXISTS "${VS_FOLDER}/${FILE}")
            message(FATAL_ERROR "File not present - ${VS_FOLDER}/${FILE} - ensure that the ${VS_DESCRIPTION} are installed with Visual Studio.")
        endif()
    endforeach()
endfunction()

#[[====================================================================================================================
    findVisualStudio
    ----------------

    Finds a Visual Studio instance, and sets CMake variables based on properties of the found instance.

        findVisualStudio(
            [VERSION <version range>]
            [PRERELEASE <ON|OFF>]
            [PRODUCTS <products>]
            [REQUIRES <vs component>...]
            PROPERTIES
                <<vswhere property> <cmake variable>>
            )
====================================================================================================================]]#
function(findVisualStudio)
    set(OPTIONS)
    set(ONE_VALUE_KEYWORDS VERSION PRERELEASE PRODUCTS)
    set(MULTI_VALUE_KEYWORDS REQUIRES PROPERTIES)

    cmake_parse_arguments(PARSE_ARGV 0 FIND_VS "${OPTIONS}" "${ONE_VALUE_KEYWORDS}" "${MULTI_VALUE_KEYWORDS}")

    set(VSWHERE_HINT_PATH "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer")

    # Accommodate WSL
    if((CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux") AND (EXISTS "/usr/bin/wslpath"))
        toolchain_read_reg_string("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion" "ProgramFilesDir (x86)" PROGRAM_FILES_X86_PATH)
        set(VSWHERE_HINT_PATH "${PROGRAM_FILES_X86_PATH}\\Microsoft Visual Studio\\Installer")
        toolchain_to_wsl_path("${VSWHERE_HINT_PATH}" VSWHERE_HINT_PATH)
    endif()

    find_program(VSWHERE_PATH
        NAMES vswhere vswhere.exe
        HINTS ${VSWHERE_HINT_PATH}
    )

    if(NOT VSWHERE_PATH)
        message(FATAL_ERROR "'vswhere' isn't found.")
    endif()

    set(VSWHERE_COMMAND ${VSWHERE_PATH} -latest)

    if(FIND_VS_PRERELEASE)
        list(APPEND VSWHERE_COMMAND -prerelease)
    endif()

    if(FIND_VS_PRODUCTS)
        list(APPEND VSWHERE_COMMAND -products ${FIND_VS_PRODUCTS})
    endif()

    if(FIND_VS_REQUIRES)
        list(APPEND VSWHERE_COMMAND -requires ${FIND_VS_REQUIRES})
    endif()

    if(FIND_VS_VERSION)
        list(APPEND VSWHERE_COMMAND -version "${FIND_VS_VERSION}")
    endif()

    message(VERBOSE "findVisualStudio: VSWHERE_COMMAND = ${VSWHERE_COMMAND}")

    execute_process(
        COMMAND ${VSWHERE_COMMAND}
        OUTPUT_VARIABLE VSWHERE_OUTPUT
    )

    message(VERBOSE "findVisualStudio: VSWHERE_OUTPUT = ${VSWHERE_OUTPUT}")

    # Matches `VSWHERE_PROPERTY` in the `VSWHERE_OUTPUT` text in the format written by vswhere.
    # The matched value is assigned to the variable `VARIABLE_NAME` in the parent scope.
    function(getVSWhereProperty VSWHERE_OUTPUT VSWHERE_PROPERTY VARIABLE_NAME)
        string(REGEX MATCH "${VSWHERE_PROPERTY}: [^\r\n]*" VSWHERE_VALUE "${VSWHERE_OUTPUT}")
        string(REPLACE "${VSWHERE_PROPERTY}: " "" VSWHERE_VALUE "${VSWHERE_VALUE}")
        set(${VARIABLE_NAME} "${VSWHERE_VALUE}" PARENT_SCOPE)
    endfunction()

    while(FIND_VS_PROPERTIES)
        list(POP_FRONT FIND_VS_PROPERTIES VSWHERE_PROPERTY)
        list(POP_FRONT FIND_VS_PROPERTIES VSWHERE_CMAKE_VARIABLE)
        getVSWhereProperty("${VSWHERE_OUTPUT}" ${VSWHERE_PROPERTY} VSWHERE_VALUE)
        set(${VSWHERE_CMAKE_VARIABLE} ${VSWHERE_VALUE} PARENT_SCOPE)
    endwhile()
endfunction()
