// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to sin(x). The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(pi/2) = sin(pi/2) = 1. It minimizes the quantity
// maximum{|sin(x) - p(x)| : x in [-pi/2,pi/2]} over all polynomials of
// degree D subject to the constraints mentioned.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    std::array<std::array<double, 6>, 5> constexpr C_SIN_EST_COEFF =
    { {
        {   // degree 3
            +1.0,
            -1.4727245910375519e-1
        },
        {   // degree 5
            +1.0,
            -1.6600599923812209e-1,
            +7.5924178409012000e-3
        },
        {   // degree 7
            +1.0,
            -1.6665578084732124e-1,
            +8.3109378830028557e-3,
            -1.8447486103462252e-4
        },
        {   // degree 9
            +1.0,
            -1.6666656235308897e-1,
            +8.3329962509886002e-3,
            -1.9805100675274190e-4,
            +2.5967200279475300e-6
        },
        {   // degree 11
            +1.0,
            -1.6666666601721269e-1,
            +8.3333303183525942e-3,
            -1.9840782426250314e-4,
            +2.7521557770526783e-6,
            -2.3828544692960918e-8
        }
    } };

    std::array<double, 5> constexpr C_SIN_EST_MAX_ERROR =
    {
        1.3481903639146e-2,  // degree 3
        1.4001209384651e-4,  // degree 5
        1.0205878939740e-6,  // degree 7
        5.2010783457846e-9,  // degree 9
        1.9323431743601e-11  // degree 11
    };
}

namespace gte
{
    // The input constraint is x in [-pi/2,pi/2]. For example a degree-3
    // estimate is
    //   float x; // in [-pi/2,pi/2]
    //   float result = SinEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T SinEstimate(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 5,
            "Invalid degree.");

        size_t constexpr select = ((Degree - 3) / 2);
        auto constexpr& coeff = C_SIN_EST_COEFF[select];
        size_t constexpr last = ((Degree - 1) / 2);
        T xsqr = x * x;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * xsqr;
        }
        poly = poly * x;
        return poly;
    }

    // The input x can be any real number. Range reduction is used to
    // generate a value y in [-pi/2,pi/2] for which sin(y) = sin(x).
    // For example a degree-3 estimate is
    //   float x;  // x any real number
    //   float result = SinEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T SinEstimateRR(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 5,
            "Invalid degree.");

        // Map x to r in [-pi,pi].
        T r = std::remainder(x, static_cast<T>(GTE_C_TWO_PI));

        // Map r to y in [-pi/2,pi/2] with sin(y) = sin(x).
        T const halfPi = static_cast<T>(GTE_C_HALF_PI);
        if (r > halfPi)
        {
            // r is in (pi/2,pi], so y = pi - r is in (-pi/2,0]
            return SinEstimate<T, Degree>(static_cast<T>(GTE_C_PI) - r);
        }
        else if (r < -halfPi)
        {
            // r is in [-pi,-pi/2), so y = -pi - r is in [0,pi/2)
            return SinEstimate<T, Degree>(static_cast<T>(-GTE_C_PI) - r);
        }
        else
        {
            // r is in [-pi/2,pi/2], y = r
            return SinEstimate<T, Degree>(r);
        }
    }

    template <typename T, size_t Degree>
    T constexpr GetSinEstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 5,
            "Invalid degree.");

        return static_cast<T>(C_SIN_EST_MAX_ERROR[(Degree - 3) / 2]);
    }
}
