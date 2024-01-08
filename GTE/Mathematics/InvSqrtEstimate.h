// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to 1/sqrt(x). The polynomial p(x) of
// degree D minimizes the quantity maximum{|1/sqrt(x) - p(x)| : x in [1,2]}
// over all polynomials of degree D.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    std::array<std::array<double, 9>, 8> constexpr C_INVSQRT_EST_COEFF =
    { {
        {   // degree 1
            +1.0,
            -2.9289321881345254e-1
        },
        {   // degree 2
            +1.0,
            -4.4539812104566801e-1,
            +1.5250490223221547e-1
        },
        {   // degree 3
            +1.0,
            -4.8703230993068791e-1,
            +2.8163710486669835e-1,
            -8.7498013749463421e-2
        },
        {   // degree 4
            +1.0,
            -4.9710061558048779e-1,
            +3.4266247597676802e-1,
            -1.9106356536293490e-1,
            +5.2608486153198797e-2
        },
        {   // degree 5
            +1.0,
            -4.9937760586004143e-1,
            +3.6508741295133973e-1,
            -2.5884890281853501e-1,
            +1.3275782221320753e-1,
            -3.2511945299404488e-2
        },
        {   // degree 6
            +1.0,
            -4.9987029229547453e-1,
            +3.7220923604495226e-1,
            -2.9193067713256937e-1,
            +1.9937605991094642e-1,
            -9.3135712130901993e-2,
            +2.0458166789566690e-2
        },
        {   // degree 7
            +1.0,
            -4.9997357250704977e-1,
            +3.7426216884998809e-1,
            -3.0539882498248971e-1,
            +2.3976005607005391e-1,
            -1.5410326351684489e-1,
            +6.5598809723041995e-2,
            -1.3038592450470787e-2
        },
        {   // degree 8
            +1.0,
            -4.9999471066120371e-1,
            +3.7481415745794067e-1,
            -3.1023804387422160e-1,
            +2.5977002682930106e-1,
            -1.9818790717727097e-1,
            +1.1882414252613671e-1,
            -4.6270038088550791e-2,
            +8.3891541755747312e-3
        }
    } };

    std::array<double, 8> constexpr C_INVSQRT_EST_MAX_ERROR =
    {
        3.7814314552702e-2,  // degree 1
        4.1953446330581e-3,  // degree 2
        5.6307702007275e-4,  // degree 3
        8.1513919990229e-5,  // degree 4
        1.2289367490981e-5,  // degree 5
        1.9001451476708e-6,  // degree 6
        2.9887737629242e-7,  // degree 7
        4.7597402907940e-8   // degree 8
    };
}

namespace gte
{
    // The input constraint is x in [1,2]. For example a degree-3 estimate is
    //   float x; // in [1,2]
    //   float result = InvSqrtEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T InvSqrtEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        size_t constexpr select = Degree - 1;
        auto constexpr& coeff = C_INVSQRT_EST_COEFF[select];
        size_t constexpr last = Degree;
        T t = x - static_cast<T>(1);  // t in [0,1]
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * t;
        }
        return poly;
    }

    // The input constraint is x > 0. Range reduction is used to generate
    // a value y in [1,2], call InvSqrtEstimate(y) and then combine the output
    // with the proper exponent to obtain the approximation. For example a
    // degree-3 estimate is
    //   float x;  // x > 0
    //   float result = InvSqrtEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T InvSqrtEstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        // Apply the reduction.
        int32_t p{};
        T y = std::frexp(x, &p);  // y in [1/2,1)
        y = static_cast<T>(2) * y;  // y in [1,2)
        --p;
        T adj = static_cast<T>(1 & p) * static_cast<T>(GTE_C_INV_SQRT_2)
            + static_cast<T>(1 & ~p) * static_cast<T>(1);
        p = -(p >> 1);

        // Evaluate the polynomial on the reduced range.
        T poly = InvSqrtEstimate<T, Degree>(y);

        // Combine the outputs.
        T result = adj * std::ldexp(poly, p);
        return result;
    }

    template <typename T, size_t Degree>
    T constexpr GetInvSqrtEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(C_INVSQRT_EST_MAX_ERROR[Degree - 1]);
    }
}
