// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <cmath>
#include <cstdint>

// These functions are recommended by the IEEE 754-2008 Standard.
namespace gtl
{
    template <typename T>
    inline T atandivpi(T const& x)
    {
        return std::atan(x) * C_INV_PI<T>;
    }

    template <typename T>
    inline T atan2divpi(T const& y, T const& x)
    {
        return std::atan2(y, x) * C_INV_PI<T>;
    }

    template <typename T>
    inline T clamp(T const& x, T const& xmin, T const& xmax)
    {
        return (x <= xmin ? xmin : (x >= xmax ? xmax : x));
    }

    template <typename T>
    inline T cospi(T const& x)
    {
        return std::cos(x * C_PI<T>);
    }

    template <typename T>
    inline T exp10(T const& x)
    {
        return std::exp(x * C_LN_10<T>);
    }

    template <typename T>
    inline T invsqrt(T const& x)
    {
        return C_<T>(1) / std::sqrt(x);
    }

    template <typename T>
    inline int32_t isign(T const& x)
    {
        return (x > C_<T>(0) ? 1 : (x < C_<T>(0) ? -1 : 0));
    }

    template <typename T>
    inline T saturate(T const& x)
    {
        return (x <= C_<T>(0) ? C_<T>(0) : (x >= C_<T>(1) ? C_<T>(1) : x));
    }

    template <typename T>
    inline T sign(T const& x)
    {
        return (x > C_<T>(0) ? C_<T>(1) : (x < C_<T>(0) ? -C_<T>(1) : C_<T>(0)));
    }

    template <typename T>
    inline T sinpi(T const& x)
    {
        return std::sin(x * C_PI<T>);
    }

    template <typename T>
    inline T sqr(T const& x)
    {
        return x * x;
    }
}
