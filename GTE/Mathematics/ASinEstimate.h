// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Approximations to asin(x) of the form f(x) = pi/2 - sqrt(1-x)*p(x), where
// the polynomial p(x) of degree D minimizes the quantity
// maximum{|acos(x)/sqrt(1-x) - p(x)| : x in [0,1]} over all polynomials of
// degree D. The identity asin(x) = pi/2 - acos(x) is used.

#include <Mathematics/ACosEstimate.h>
#include <Mathematics/Constants.h>
#include <array>
#include <cstddef>

namespace gte
{
    std::array<double, 8> constexpr C_ASIN_EST_MAX_ERROR =
    {
        9.0128265558586e-3,  // degree 1
        8.1851275863202e-4,  // degree 2
        8.8200141836567e-5,  // degree 3
        1.0563052499871e-5,  // degree 4
        1.3535063235066e-6,  // degree 5
        1.8169471743823e-7,  // degree 6
        2.5231622315797e-8,  // degree 7
        3.5952707963527e-9   // degree 8
    };
}

namespace gte
{
    // The input constraint is x in [0,1]. For example a degree-3 estimate is
    //   float x; // in [0,1]
    //   float result = ASinEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ASinEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(GTE_C_HALF_PI) - ACosEstimate<T, Degree>(x);
    };

    template <typename T, size_t Degree>
    T constexpr GetASinEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(C_ASIN_EST_MAX_ERROR[Degree - 1]);
    }
}
