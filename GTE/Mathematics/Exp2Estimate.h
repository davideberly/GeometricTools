// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to 2^x. The polynomial p(x) of
// degree D minimizes the quantity maximum{|2^x - p(x)| : x in [0,1]}
// over all polynomials of degree D.

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    std::array<std::array<double, 8>, 7> constexpr C_EXP2_EST_COEFF =
    { {
        {   // degree 1
            1.0,
            1.0
        },
        {   // degree 2
            1.0,
            6.5571332605741528e-1,
            3.4428667394258472e-1
        },
        {   // degree 3
            1.0,
            6.9589012084456225e-1,
            2.2486494900110188e-1,
            7.9244930154334980e-2
        },
        {   // degree 4
            1.0,
            6.9300392358459195e-1,
            2.4154981722455560e-1,
            5.1744260331489045e-2,
            1.3701998859367848e-2
        },
        {   // degree 5
            1.0,
            6.9315298010274962e-1,
            2.4014712313022102e-1,
            5.5855296413199085e-2,
            8.9477503096873079e-3,
            1.8968500441332026e-3
        },
        {   // degree 6
            1.0,
            6.9314698914837525e-1,
            2.4023013440952923e-1,
            5.5481276898206033e-2,
            9.6838443037086108e-3,
            1.2388324048515642e-3,
            2.1892283501756538e-4
        },
        {   // degree 7
            1.0,
            6.9314718588750690e-1,
            2.4022637363165700e-1,
            5.5505235570535660e-2,
            9.6136265387940512e-3,
            1.3429234504656051e-3,
            1.4299202757683815e-4,
            2.1662892777385423e-5
        }
    } };

    std::array<double, 7> constexpr C_EXP2_EST_MAX_ERROR =
    {
        8.6071332055935e-2,  // degree 1
        3.8132476831059e-3,  // degree 2
        1.4694877755229e-4,  // degree 3
        4.7617792662269e-6,  // degree 4
        1.3162098788655e-7,  // degree 5
        3.1590552396211e-9,  // degree 6
        6.7157390759576e-11  // degree 7
    };
}

namespace gte
{
    // The input constraint is x in [0,1]. For example a degree-3 estimate is
    //   float x; // in [0,1]
    //   float result = Exp2Estimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T Exp2Estimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        size_t constexpr select = Degree - 1;
        auto constexpr& coeff = C_EXP2_EST_COEFF[select];
        size_t constexpr last = Degree;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * x;
        }
        return poly;
    }

    // The input x can be any real number. Range reduction is used to generate
    // a value y in [0,1], call Exp2Estimate(y) and then combine the output
    // with the proper exponent to obtain the approximation. For example a
    // degree-3 estimate is
    //   float x;  // x >= 0
    //   float result = Exp2EstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T Exp2EstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        T p = std::floor(x);
        T y = x - p;
        T poly = Exp2Estimate<T, Degree>(y);
        int32_t power = static_cast<int32_t>(static_cast<double>(p));
        T result = std::ldexp(poly, power);
        return result;
    }

    template <typename T, size_t Degree>
    T constexpr GetExp2EstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 7,
            "Invalid degree.");

        return static_cast<T>(C_EXP2_EST_MAX_ERROR[Degree - 1]);
    }
}
