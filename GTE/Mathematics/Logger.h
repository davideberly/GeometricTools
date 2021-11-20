// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2021.11.11

#pragma once

#include <stdexcept>
#include <string>

// Generate exceptions about unexpected conditions. The messages can be
// intercepted in a 'catch' block and processed as desired. The 'exception'
// in the macros is one of the standard exceptions provided by C++. You can
// also derive your own exception class from those provided by C++ and pass
// those via the macros.

// The report uses the current source file, function and line on which the
// macro is expanded.
#define GTE_ASSERT(condition, exception, message) \
if (!(condition)) { throw exception(std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): " + message + "\n"); }

#define GTE_ERROR(exception, message) \
{ throw exception(std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): " + message + "\n"); }

// The report uses the specified source file, function and line. The file
// and function are type 'char const*' and the line is type 'int'.
#define GTE_ASSERT_INDIRECT(condition, exception, file, function, line, message) \
if (!(condition)) { throw exception(std::string(file) + "(" + std::string(function) + "," + std::to_string(line) + "): " + message + "\n"); }

#define GTE_ERROR_INDIRECT(exception, file, function, line, message) \
{ throw exception(std::string(file) + "(" + std::string(function) + "," + std::to_string(line) + "): " + message + "\n"); }

#define LogAssert(condition, message) \
GTE_ASSERT(condition, std::runtime_error, message)

#define LogError(message) \
GTE_ERROR(std::runtime_error, message);
