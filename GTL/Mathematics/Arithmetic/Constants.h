// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Utility/TypeTraits.h>
#include <cstdint>

namespace gtl
{
    // Common integer-valued constants; for example, zero is represented
    // by C_<T>(0) for T in {float, double, BSNumber<*>, BSRational<*>}.
    template <typename T>
    T const C_(int32_t number)
    {
        return static_cast<T>(number);
    }

    // Common fractional constants of the form n/d where n and d are integers.
    // The first function supports T in { float, double, BSRational<*>. If T
    // is BSRational<*>, the constant is an exact representation of the number
    // n/d. The second function supports BSNumber<*>. Generally, the constant
    // is not an exact representation because the floating-point division of
    // floating-point n by floating-point d has rounding errors. If there is
    // no rounding error, say, in the case when d is a power of 2, the
    // representation is exact.
    template <typename T, IsDivisionType<T> = 0>
    T const C_(int32_t n, int32_t d)
    {
        return
            static_cast<T>(static_cast<double>(n)) /
            static_cast<T>(static_cast<double>(d));
    }

    template <typename T, IsNotDivisionType<T> = 0>
    T const C_(int32_t n, int32_t d)
    {
        return static_cast<T>(static_cast<double>(n) / static_cast<double>(d));
    }

    // Named constants involving pi.
    template <typename T> const T C_PI = static_cast<T>(3.1415926535897931);
    template <typename T> const T C_PI_DIV_2 = static_cast<T>(1.5707963267948966);
    template <typename T> const T C_PI_DIV_4 = static_cast<T>(0.7853981633974483);
    template <typename T> const T C_TWO_PI = static_cast<T>(6.2831853071795862);
    template <typename T> const T C_INV_PI = static_cast<T>(0.3183098861837907);
    template <typename T> const T C_INV_TWO_PI = static_cast<T>(0.1591549430918953);
    template <typename T> const T C_INV_HALF_PI = static_cast<T>(0.6366197723675813);

    // Named constants for conversions between degrees and radians.
    template <typename T> const T C_DEG_TO_RAD = static_cast<T>(0.0174532925199433);
    template <typename T> const T C_RAD_TO_DEG = static_cast<T>(57.295779513082321);

    // Named constants that are appear less frequently in the code.
    template <typename T> const T C_SQRT_2 = static_cast<T>(1.4142135623730951);
    template <typename T> const T C_INV_SQRT_2 = static_cast<T>(0.7071067811865475);
    template <typename T> const T C_SQRT_3 = static_cast<T>(1.7320508075688772);
    template <typename T> const T C_INV_SQRT_3 = static_cast<T>(0.57735026918962576);
    template <typename T> const T C_LN_2 = static_cast<T>(0.6931471805599453);
    template <typename T> const T C_INV_LN_2 = static_cast<T>(1.4426950408889634);
    template <typename T> const T C_LN_10 = static_cast<T>(2.3025850929940459);
    template <typename T> const T C_INV_LN_10 = static_cast<T>(0.43429448190325176);
}
