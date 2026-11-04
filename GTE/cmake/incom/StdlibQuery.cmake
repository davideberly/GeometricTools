#=============================================================================
# StdlibQuery.cmake
#
# 1. Tries to compile three different tiny source files to find out what
#    standard library is being used
#    (this can be useful in other places in CMake, where one might depend
#    on the platform being used and not just the compiler)
#
# Usage:
#   include(StdlibQuery.cmake)
##
# This will define:
#   USING_LIBSTDCXX
#   USING_LIBCXX
#   USING_MSVC_STL
#
#   Note that only one of the above will be set to truthy value. The remaining two will be false.
#   Triggers FATAL_ERROR on configure should more than one of the above evaluate to truthy value.
#
#=============================================================================

# StdlibQuery.cmake

get_property(__STDLIB_ALREADY_RUN GLOBAL PROPERTY STDLIB_DETECTED SET)
if(__STDLIB_ALREADY_RUN)
    unset(__STDLIB_ALREADY_RUN)
    return()
endif()


include(CheckCXXSourceCompiles)

# libc++
check_cxx_source_compiles("
#include <cstddef>
#ifdef _LIBCPP_VERSION
int main() { return 0; }
#else
#error
#endif
"    USING_LIBCXX)

# libstdc++
check_cxx_source_compiles("
#include <cstddef>
#ifdef _GLIBCXX_RELEASE
int main() { return 0; }
#else
#error
#endif
"    USING_LIBSTDCXX)

# MSVC STL
check_cxx_source_compiles("
#include <yvals_core.h>
#ifdef _MSVC_STL_VERSION
int main() { return 0; }
#else
#error
#endif
"    USING_MSVC_STL)


# ---- Validation: exactly one must be true ----
set(__STDLIB_COUNT 0)
if(USING_LIBCXX)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()
if(USING_LIBSTDCXX)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()
if(USING_MSVC_STL)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()

if(__STDLIB_COUNT EQUAL 0)
    message(FATAL_ERROR
        "Failed to detect the C++ standard library.\n"
        "None of libc++, libstdc++, or MSVC STL were detected.\n"
        "Compiler: ${CMAKE_CXX_COMPILER}\n"
        "Compiler frontend variant: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}\n"
        "Please ensure a supported standard library is available."
    )
elseif(__STDLIB_COUNT GREATER 1)
    message(FATAL_ERROR
        "Multiple C++ standard libraries detected (this should never ever happen).\n"
        "Detected:\n"
        "  libc++      = ${USING_LIBCXX}\n"
        "  libstdc++   = ${USING_LIBSTDCXX}\n"
        "  MSVC STL   = ${USING_MSVC_STL}\n"
        "Please report this configuration as a bug."
    )
endif()

# ---- Post message about the result
if(USING_LIBSTDCXX)
  message(STATUS "Using libstdc++")
elseif(USING_LIBCXX)
  message(STATUS "Using libc++")
elseif(USING_MSVC_STL)
  message(STATUS "Using MSVC STL")
endif()

set_property(GLOBAL PROPERTY STDLIB_DETECTED TRUE)
unset(__STDLIB_COUNT)
unset(__STDLIB_ALREADY_RUN)
