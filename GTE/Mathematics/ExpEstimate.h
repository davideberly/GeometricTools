// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to exp(x) of the form f(x) =
// p(x/log(2)), where log(2) is the natural logarithm of 2 and the polynomial
// p(y) of degree D minimizes the quantity maximum{|2^y - p(y)| : y in [0,1]}
// over all polynomials of degree D. The identity exp(x) = 2^{x/log(2)} is
// used.

#include <Mathematics/Constants.h>
#include <Mathematics/Exp2Estimate.h>
#include <array>
#include <cstddef>

namespace gte
{
    std::array<double, 7> constexpr C_EXP_EST_MAX_ERROR =
    {
        8.6071332055935e-2,  // degree 1
        3.8132476831059e-3,  // degree 2
        1.4694877755229e-4,  // degree 3
        4.7617792662269e-6,  // degree 4
        1.3162098766451e-7,  // degree 5
        3.1590550175765e-9,  // degree 6
        6.7157168714971e-11  // degree 7
    };
}

namespace gte
{
    // The input constraint is x in [0,log(2)], where log(2) is the natural
    // logarithm of 2. For example a degree-3 estimate is
    //   float x; // in [0,log(2)]
    //   float result = ExpEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ExpEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        return Exp2Estimate<T, Degree>(x * static_cast<T>(GTE_C_INV_LN_2));
    }

    // The input x can be any real number. Range reduction is used to generate
    // a value y in [0,log(2)], call ExpEstimate(y) and then combine the
    // output with the proper exponent to obtain the approximation. For
    // example,
    //   float x;  // x >= 0
    //   float result = ExpEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ExpEstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        return Exp2EstimateRR<T, Degree>(x * static_cast<T>(GTE_C_INV_LN_2));
    }

    template <typename T, size_t Degree>
    T constexpr GetExpEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        return static_cast<T>(C_EXP_EST_MAX_ERROR[Degree - 1]);
    }
}
