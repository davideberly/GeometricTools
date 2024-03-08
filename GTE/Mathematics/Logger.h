// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.05.07

#pragma once

#include <stdexcept>
#include <string>

// If exceptions are enabled, throw the exception passed in, otherwise call std::terminate.
#if (defined(__cpp_exceptions) || defined(__EXCEPTIONS) || (defined(_MSC_VER) && defined(_CPPUNWIND))) && !defined(GTE_NO_EXCEPTIONS)
    #define GTE_THROW_OR_TERMINATE(exception, message) throw exception(message)
#else
    #define GTE_THROW_OR_TERMINATE(exception, message) std::terminate()
#endif

// Generate exceptions about unexpected conditions. The messages can be
// intercepted in a 'catch' block and processed as desired. The 'exception'
// in the macros is one of the standard exceptions provided by C++. You can
// also derive your own exception class from those provided by C++ and pass
// those via the macros.

// The report uses the current source file, function and line on which the
// macro is expanded.
#define GTE_ASSERT(condition, exception, message) \
if (!(condition)) { GTE_THROW_OR_TERMINATE(exception, std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): " + message + "\n"); }

#define GTE_ERROR(exception, message) \
{ GTE_THROW_OR_TERMINATE(exception, std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): " + message + "\n"); }

// The report uses the specified source file, function and line. The file
// and function are type 'char const*' and the line is type 'int32_t'.
#define GTE_ASSERT_INDIRECT(condition, exception, file, function, line, message) \
if (!(condition)) { GTE_THROW_OR_TERMINATE(exception, std::string(file) + "(" + std::string(function) + "," + std::to_string(line) + "): " + message + "\n"); }

#define GTE_ERROR_INDIRECT(exception, file, function, line, message) \
{ GTE_THROW_OR_TERMINATE(exception, std::string(file) + "(" + std::string(function) + "," + std::to_string(line) + "): " + message + "\n"); }

#define LogAssert(condition, message) \
GTE_ASSERT(condition, std::runtime_error, message)

#define LogError(message) \
GTE_ERROR(std::runtime_error, message);
