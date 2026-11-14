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

#[[====================================================================================================================
    toolchain_read_reg_string
    -------------------------

    Reads a string value from the Windows registry using reg.exe via WSL.

        toolchain_read_reg_string(
            <reg_key>
            <reg_value>
            <output_variable>
        )

    Note: Not supported for consumption outside of the toolchain files.
====================================================================================================================]]#
function(toolchain_read_reg_string REG_KEY REG_VALUE OUTPUT_VARIABLE)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        message(FATAL_ERROR "toolchain_read_reg_string should not be used on Windows platforms.")
    endif()

    find_program(REG_EXE_PATH NAMES reg.exe)
    if(NOT REG_EXE_PATH)
        message(FATAL_ERROR "reg.exe not found - cannot read Windows registry from WSL.")
    endif()

    execute_process(
        COMMAND ${REG_EXE_PATH} QUERY ${REG_KEY} /v ${REG_VALUE}
        OUTPUT_VARIABLE REG_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REGEX REPLACE ".*REG_SZ[ \t]+(.*)$" "\\1" REG_OUTPUT "${REG_OUTPUT}")
    message(VERBOSE "toolchain_read_reg_string: VALUE = ${REG_OUTPUT}")
    set(${OUTPUT_VARIABLE} "${REG_OUTPUT}" PARENT_SCOPE)
endfunction()

#[[====================================================================================================================
    toolchain_to_wsl_path
    ---------------------

    Converts a Windows path to a WSL path.

        toolchain_to_wsl_path(
            <input_path>
            <output_variable>
        )

    Note: Not supported for consumption outside of the toolchain files.
====================================================================================================================]]#
function(toolchain_to_wsl_path INPUT_PATH OUTPUT_VARIABLE)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        message(FATAL_ERROR "toolchain_to_wsl_path should not be used on Windows platforms.")
    endif()

    if(NOT (EXISTS "/usr/bin/wslpath"))
        message(FATAL_ERROR "wslpath not found - cannot convert Windows path to WSL path.")
    endif()

    execute_process(COMMAND /usr/bin/wslpath -u ${INPUT_PATH}
            OUTPUT_VARIABLE WSL_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${OUTPUT_VARIABLE} "${WSL_PATH}" PARENT_SCOPE)
endfunction()
