// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to atan(x). The polynomial p(x) of
// degree D has only odd-power terms, is required to have linear term x,
// and p(1) = atan(1) = pi/4. It minimizes the quantity
// maximum{|atan(x) - p(x)| : x in [-1,1]} over all polynomials of
// degree D subject to the constraints mentioned.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    std::array<std::array<double, 7>, 6> constexpr C_ATAN_EST_COEFF =
    { {
        {   // degree 3
            +1.0,
            -2.1460183660255172e-1
        },
        {   // degree 5
            +1.0,
            -3.0189478312144946e-1,
            +8.7292946518897740e-2
        },
        {   // degree 7
            +1.0,
            -3.2570157599356531e-1,
            +1.5342994884206673e-1,
            -4.2330209451053591e-2
        },
        {   // degree 9
            +1.0,
            -3.3157878236439586e-1,
            +1.8383034738018011e-1,
            -8.9253037587244677e-2,
            +2.2399635968909593e-2
        },
        {   // degree 11
            +1.0,
            -3.3294527685374087e-1,
            +1.9498657165383548e-1,
            -1.1921576270475498e-1,
            +5.5063351366968050e-2,
            -1.2490720064867844e-2
        },
        {   // degree 13
            +1.0,
            -3.3324998579202170e-1,
            +1.9856563505717162e-1,
            -1.3374657325451267e-1,
            +8.1675882859940430e-2,
            -3.5059680836411644e-2,
            +7.2128853633444123e-3
        }
    } };

    std::array<double, 6> constexpr C_ATAN_EST_MAX_ERROR =
    {
        1.5970326392625e-2,  // degree 3
        1.3509832247375e-3,  // degree 5
        1.5051227215525e-4,  // degree 7
        1.8921598624725e-5,  // degree 9
        2.5477725020825e-6,  // degree 11
        3.5859106295450e-7   // degree 13
    };
}

namespace gte
{
    // The input constraint is x in [-1,1]. For example a degree-3 estimate is
    //   float x; // in [-1,1]
    //   float result = ATanEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ATanEstimate(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= (Degree - 1) / 2 && (Degree - 1) / 2 <= 6,
            "Invalid degree.");

        size_t constexpr select = (Degree - 3) / 2;
        auto constexpr& coeff = C_ATAN_EST_COEFF[select];
        size_t constexpr last = (Degree - 1) / 2;
        T xsqr = x * x;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * xsqr;
        }
        poly = poly * x;
        return poly;
    }

    // The input x can be any real number. Range reduction is used
    // via the identities atan(x) = pi/2 - atan(1/x) for x > 0 and
    // atan(x) = -pi/2 - atan(1/x) for x < 0. For example,
    //   float x;  // x any real number
    //   float result = ATanEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ATanEstimateRR(T x)
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= (Degree - 1) / 2 && (Degree - 1) / 2 <= 6,
            "Invalid degree.");

        T const one = static_cast<T>(1);
        if (std::fabs(x) <= one)
        {
            return ATanEstimate<T, Degree>(x);
        }
        else if (x > one)
        {
            return static_cast<T>(GTE_C_HALF_PI) - ATanEstimate<T, Degree>(one / x);
        }
        else
        {
            return static_cast<T>(-GTE_C_HALF_PI) - ATanEstimate<T, Degree>(one / x);
        }
    }

    template <typename T, size_t Degree>
    T constexpr GetATanEstimateMaxError()
    {
        static_assert(
            (Degree & 1) == 1 && 1 <= (Degree - 1) / 2 && (Degree - 1) / 2 <= 6,
            "Invalid degree.");

        return static_cast<T>(C_ATAN_EST_MAX_ERROR[(Degree - 3) / 2]);
    }
}
