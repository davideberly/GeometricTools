// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to cos(x). The polynomial p(x) of
// degree D has only even-power terms, is required to have constant term 1,
// and p(pi/2) = cos(pi/2) = 0. It minimizes the quantity
// maximum{|cos(x) - p(x)| : x in [-pi/2,pi/2]} over all polynomials of
// degree D subject to the constraints mentioned.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    std::array<std::array<double, 6>, 5> constexpr C_COS_EST_COEFF =
    { {
        {   // degree 2
            +1.0,
            -4.0528473456935105e-1
        },
        {   // degree 4
            +1.0,
            -4.9607181958647262e-1,
            +3.6794619653489236e-2
        },
        {   // degree 6
            +1.0,
            -4.9992746217057404e-1,
            +4.1493920348353308e-2,
            -1.2712435011987822e-3
        },
        {   // degree 8
            +1.0,
            -4.9999925121358291e-1,
            +4.1663780117805693e-2,
            -1.3854239405310942e-3,
            +2.3154171575501259e-5
        },
        {   // degree 10
            +1.0,
            -4.9999999508695869e-1,
            +4.1666638865338612e-2,
            -1.3888377661039897e-3,
            +2.4760495088926859e-5,
            -2.6051615464872668e-7
        }
    } };

    std::array<double, 5> constexpr C_COS_EST_MAX_ERROR =
    {
        5.6009595954128e-2,  // degree 2
        9.1879932449727e-4,  // degree 4
        9.2028470144446e-6,  // degree 6
        5.9804535233743e-8,  // degree 8
        2.7008567604626e-10  // degree 10
    };
}

namespace gte
{
    // The input constraint is x in [-pi/2,pi/2]. For example a degree-4
    // estimate is
    //   float x; // in [-pi/2,pi/2]
    //   float result = CosEstimate<float, 4>(x);
    template <typename T, size_t Degree>
    inline T CosEstimate(T x)
    {
        static_assert(
            (Degree & 1) == 0 && 1 <= (Degree / 2) && (Degree / 2) <= 5,
            "Invalid degree.");

        size_t constexpr select = (Degree - 2) / 2;
        auto constexpr& coeff = C_COS_EST_COEFF[select];
        size_t constexpr last = (Degree / 2);
        T xsqr = x * x;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * xsqr;
        }
        return poly;
    }

    // The input x can be any real number. Range reduction is used to generate
    // a value y in [-pi/2,pi/2] and a sign s for which cos(y) = s * cos(x).
    // For example a degree-4 estimate is
    //   float x;  // x any real number
    //   float result = CosEstimateRR<float, 4>(x);
    template <typename T, size_t Degree>
    inline T CosEstimateRR(T x)
    {
        static_assert(
            (Degree & 1) == 0 && 1 <= (Degree / 2) && (Degree / 2) <= 5,
            "Invalid degree.");

        // Map x to r in [-pi,pi].
        T r = std::remainder(x, static_cast<T>(GTE_C_TWO_PI));

        // Map r to y in [-pi/2,pi/2] with cos(y) = sign * cos(x).
        T const halfPi = static_cast<T>(GTE_C_HALF_PI);
        if (r > halfPi)
        {
            // r is in (pi/2,pi], so y = pi - r is in (-pi/2,0], sign = -1
            return -CosEstimate<T, Degree>(static_cast<T>(GTE_C_PI) - r);
        }
        else if (r < -halfPi)
        {
            // r is in [-pi,-pi/2), so y = -pi - r is in [0,pi/2), sign = -1
            return -CosEstimate<T, Degree>(static_cast<T>(-GTE_C_PI) - r);
        }
        else
        {
            // r is in [-pi/2,pi/2], y = r, sign = +1
            return CosEstimate<T, Degree>(r);
        }
    }

    template <typename T, size_t Degree>
    T constexpr GetCosEstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 0 && 1 <= (Degree / 2) && (Degree / 2) <= 5,
            "Invalid degree.");

        return static_cast<T>(C_COS_EST_MAX_ERROR[(Degree - 2) / 2]);
    }
}
