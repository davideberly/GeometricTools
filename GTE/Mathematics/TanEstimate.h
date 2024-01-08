// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to tan(x).  The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(pi/4) = tan(pi/4) = 1.  It minimizes the quantity
// maximum{|tan(x) - p(x)| : x in [-pi/4,pi/4]} over all polynomials of
// degree D subject to the constraints mentioned.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    std::array<std::array<double, 7>, 6> constexpr C_TAN_EST_COEFF =
    { {
        {   // degree 3
            1.0,
            4.4295926544736286e-1
        },
        {   // degree 5
            1.0,
            3.1401320403542421e-1,
            2.0903948109240345e-1
        },
        {   // degree 7
            1.0,
            3.3607213284422555e-1,
            1.1261037305184907e-1,
            9.8352099470524479e-2
        },
        {   // degree 9
            1.0,
            3.3299232843941784e-1,
            1.3747843432474838e-1,
            3.7696344813028304e-2,
            4.6097377279281204e-2
        },
        {   // degree 11
            1.0,
            3.3337224456224224e-1,
            1.3264516053824593e-1,
            5.8145237645931047e-2,
            1.0732193237572574e-2,
            2.1558456793513869e-2
        },
        {   // degree 13
            1.0,
            3.3332916426394554e-1,
            1.3343404625112498e-1,
            5.3104565343119248e-2,
            2.5355038312682154e-2,
            1.8253255966556026e-3,
            1.0069407176615641e-2
        }
    } };

    std::array<double, 6> constexpr C_TAN_EST_MAX_ERROR =
    {
        1.1661892256205e-2,  // degree 3
        5.8431854390146e-4,  // degree 5
        3.5418688397793e-5,  // degree 7
        2.2988173248307e-6,  // degree 9
        1.5426258070939e-7,  // degree 11
        1.0550265105991e-8   // degree 13
    };
}

namespace gte
{
    // The input constraint is x in [-pi/4,pi/4]. For example a degree-3
    // estimate is
    //   float x; // in [-pi/4,pi/4]
    //   float result = TanEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T TanEstimate(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 6,
            "Invalid degree.");

        size_t constexpr select = ((Degree - 3) / 2);
        auto constexpr& coeff = C_TAN_EST_COEFF[select];
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

    // The input x can be any real number. Range reduction is used to generate
    // a value y in [-pi/2,pi/2]. If |y| <= pi/4, then the polynomial is
    // evaluated. If y in (pi/4,pi/2), set z = y - pi/4 and use the identity
    //   tan(y) = tan(z + pi/4) = [1 + tan(z)]/[1 - tan(z)]
    // If y in (-pi/2,-pi/4), set z = y + pi/4 and use the identity
    //   tan(y) = tan(z - pi/4) = -[1 - tan(z)]/[1 + tan(z)]
    // Be careful when evaluating at y nearly pi/2, because tan(y) becomes
    // infinite. For example a degree-3 estimate is
    //   float x;  // x any real number
    //   float result = TanEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T TanEstimateRR(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 6,
            "Invalid degree.");

        // Map x to r in [-pi,pi].
        T const pi = static_cast<T>(GTE_C_PI);
        T r = std::remainder(x, static_cast<T>(pi));

        // Map r to y in [-pi/2,pi/2] with tan(y) = tan(r).
        T const halfPi = static_cast<T>(GTE_C_HALF_PI);
        T y{};
        if (r > halfPi)
        {
            y = r - pi;
        }
        else if (r < -halfPi)
        {
            y = r + pi;
        }
        else
        {
            y = r;
        }

        T const quarterPi = static_cast<T>(GTE_C_QUARTER_PI);
        T const one = static_cast<T>(1);
        if (std::fabs(y) <= quarterPi)
        {
            return TanEstimate<T, Degree>(y);
        }
        else if (y > quarterPi)
        {
            T poly = TanEstimate<T, Degree>(y - quarterPi);
            return (one + poly) / (one - poly);
        }
        else
        {
            T poly = TanEstimate<T, Degree>(y + quarterPi);
            return (-one + poly) / (one + poly);
        }
    }

    template <typename T, size_t Degree>
    T constexpr GetTanEstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= ((Degree - 1) / 2) && ((Degree - 1) / 2) <= 6,
            "Invalid degree.");

        return static_cast<T>(C_TAN_EST_MAX_ERROR[(Degree - 3) / 2]);
    }
}
