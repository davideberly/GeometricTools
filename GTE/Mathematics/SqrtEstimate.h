// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to sqrt(x). The polynomial p(x) of
// degree D minimizes the quantity maximum{|sqrt(x) - p(x)| : x in [1,2]}
// over all polynomials of degree D.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    std::array<std::array<double, 9>, 8> constexpr C_SQRT_EST_COEFF =
    { {
        {   // degree 1
            +1.0,
            +4.1421356237309505e-1
        },
        {   // degree 2
            +1.0,
            +4.8563183076125260e-1,
            -7.1418268388157458e-2
        },
        {   // degree 3
            +1.0,
            +4.9750045320242231e-1,
            -1.0787308044477850e-1,
            +2.4586189615451115e-2
        },
        {   // degree 4
            +1.0,
            +4.9955939832918816e-1,
            -1.2024066151943025e-1,
            +4.5461507257698486e-2,
            -1.0566681694362146e-2
        },
        {   // degree 5
            +1.0,
            +4.9992197660031912e-1,
            -1.2378506719245053e-1,
            +5.6122776972699739e-2,
            -2.3128836281145482e-2,
            +5.0827122737047148e-3
        },
        {   // degree 6
            +1.0,
            +4.9998616695784914e-1,
            -1.2470733323278438e-1,
            +6.0388587356982271e-2,
            -3.1692053551807930e-2,
            +1.2856590305148075e-2,
            -2.6183954624343642e-3
        },
        {   // degree 7
            +1.0,
            +4.9999754817809228e-1,
            -1.2493243476353655e-1,
            +6.1859954146370910e-2,
            -3.6091595023208356e-2,
            +1.9483946523450868e-2,
            -7.5166134568007692e-3,
            +1.4127567687864939e-3
        },
        {   // degree 8
            +1.0,
            +4.9999956583056759e-1,
            -1.2498490369914350e-1,
            +6.2318494667579216e-2,
            -3.7982961896432244e-2,
            +2.3642612312869460e-2,
            -1.2529377587270574e-2,
            +4.5382426960713929e-3,
            -7.8810995273670414e-4
        }
    } };

    std::array<double, 8> constexpr C_SQRT_EST_MAX_ERROR =
    {
        1.7766952966369e-2,  // degree 1
        1.1795695163111e-3,  // degree 2
        1.1309620116485e-4,  // degree 3
        1.2741170151820e-5,  // degree 4
        1.5725569051384e-6,  // degree 5
        2.0584162152560e-7,  // degree 6
        2.8072338675856e-8,  // degree 7
        3.9468401880072e-9   // degree 8
    };
}

namespace gte
{
    // The input constraint is x in [1,2]. For example a degree-3 estimate is
    //   float x; // in [1,2]
    //   float result = SqrtEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T SqrtEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        size_t constexpr select = Degree - 1;
        auto constexpr& coeff = C_SQRT_EST_COEFF[select];
        size_t constexpr last = Degree;
        T t = x - static_cast<T>(1);  // t in [0,1]
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * t;
        }
        return poly;
    }

    // The input constraint is x >= 0. Range reduction is used to generate a
    // value y in [0,1], call SqrtEstimate(y) and then combine the output with
    // the proper exponent to obtain the approximation. For example a degree-3
    // estimate is
    //   float x;  // x >= 0
    //   float result = SqrtEstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T SqrtEstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        // Apply the reduction.
        int32_t p{};
        T y = std::frexp(x, &p);  // y in [1/2,1)
        y = static_cast<T>(2) * y;   // y in [1,2)
        --p;
        T adj = static_cast<T>(1 & p) * static_cast<T>(GTE_C_SQRT_2)
            + static_cast<T>(1 & ~p) * static_cast<T>(1);
        p >>= 1;

        // Evaluate the polynomial on the reduced range.
        T poly = SqrtEstimate<T, Degree>(y);

        // Combine the outputs.
        T result = adj * std::ldexp(poly, p);
        return result;
    }

    template <typename T, size_t Degree>
    T constexpr GetSqrtEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(C_SQRT_EST_MAX_ERROR[Degree - 1]);
    }
}
