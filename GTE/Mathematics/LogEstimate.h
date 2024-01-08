// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to log(x) of the form f(x) =
// p(x)*log(2), where log(2) is the natural logarithm of 2 and the
// polynomial p(x) of degree D minimizes the quantity
// maximum{|log2(x) - p(x)| : x in [1,2]} over all polynomials of degree D.
// The identity log(x) = log2(x)*log(2) is used.

#include <Mathematics/Constants.h>
#include <Mathematics/Log2Estimate.h>
#include <cstddef>

namespace gte
{
    // The input constraint is x in [1,2]. For example a degree-3 estimate is
    //   float x; // in [1,2]
    //   float result = LogEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T LogEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return Log2Estimate<T, Degree>(x) * static_cast<T>(GTE_C_LN_2);
    }

    // The input constraint is x > 0. Range reduction is used to generate
    // a value y in [1,2], call LogEstimate(y) and then add the exponent for
    // the power of two in the binary scientific representation of x. For
    // example a degree-3 estimate is
    //   float x;  // x > 0
    //   float result = LogEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T LogEstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return Log2EstimateRR<T, Degree>(x) * static_cast<T>(GTE_C_LN_2);
    }

    template <typename T, size_t Degree>
    T constexpr GetLogEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return GetLog2EstimateMaxError<T, Degree>();
    }
}
