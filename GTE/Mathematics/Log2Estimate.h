// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Minimax polynomial approximations to log2(x).  The polynomial p(x) of
// degree D minimizes the quantity maximum{|log2(x) - p(x)| : x in [1,2]}
// over all polynomials of degree D.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    std::array<std::array<double, 8>, 8> constexpr C_LOG2_EST_COEFF =
    { {
        {   // degree 1
            +1.0
        },
        {   // degree 2
            +1.3465553856377803,
            -3.4655538563778032e-1
        },
        {   // degree 3
            +1.4228653756681227,
            -5.8208556916449616e-1,
            +1.5922019349637218e-1
        },
        {   // degree 4
            +1.4387257478171547,
            -6.7778401359918661e-1,
            +3.2118898377713379e-1,
            -8.2130717995088531e-2
        },
        {   // degree 5
            +1.4419170408633741,
            -7.0909645927612530e-1,
            +4.1560609399164150e-1,
            -1.9357573729558908e-1,
            +4.5149061716699634e-2
        },
        {   // degree 6
            +1.4425449435950917,
            -7.1814525675038965e-1,
            +4.5754919692564044e-1,
            -2.7790534462849337e-1,
            +1.2179791068763279e-1,
            -2.5841449829670182e-2
        },
        {   // degree 7
            +1.4426664401536078,
            -7.2055423726162360e-1,
            +4.7332419162501083e-1,
            -3.2514018752954144e-1,
            +1.9302965529095673e-1,
            -7.8534970641157997e-2,
            +1.5209108363023915e-2
        },
        {   // degree 8
            +1.4426896453621882,
            -7.2115893912535967e-1,
            +4.7861716616785088e-1,
            -3.4699935395019565e-1,
            +2.4114048765477492e-1,
            -1.3657398692885181e-1,
            +5.1421382871922106e-2,
            -9.1364020499895560e-3
        }
    } };

    std::array<double, 8> constexpr C_LOG2_EST_MAX_ERROR =
    {
        8.6071332055935e-2,  // degree 1
        7.6362868906659e-3,  // degree 2
        8.7902902652948e-4,  // degree 3
        1.1318551356388e-4,  // degree 4
        1.5521274483455e-5,  // degree 5
        2.2162052037978e-6,  // degree 6
        3.2546558681457e-7,  // degree 7
        4.8798286744756e-8   // degree 8
    };
}

namespace gte
{
    // The input constraint is x in [1,2]. For example a degree-3 estimate is
    //   float x; // in [1,2]
    //   float result = Log2Estimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T Log2Estimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        size_t constexpr select = Degree - 1;
        auto constexpr& coeff = C_LOG2_EST_COEFF[select];
        size_t constexpr last = Degree - 1;
        T t = x - static_cast<T>(1);  // t in [0,1]
        T poly = static_cast<T>(coeff[last]);
#if defined(GTE_USE_MSWINDOWS)
        // When Degree is 1, MSVS 2019 generates warning C6294: 'Ill-defined
        // loop: initial condition does not satisfy test. Loop body not
        // executed.' By design of the template function, the loop must be
        // skipped because p(t) = coeff[0]*t, in which case 'last' is 0. The
        // value 'index = last - 1' wraps around to the maximum size_t number,
        // but this is irrelevant because 'i' is initialized to 0 and
        // 'i < last' is '0 < 0', which is false and the loop is skipped. The
        // optimizing compiler should not generate any code at all for the
        // loop.
#pragma warning(push)
#pragma warning(disable: 6294)
#endif
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * t;

        }
        poly = poly * t;
        return poly;
    }
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(pop)
#endif

    // The input constraint is x > 0. Range reduction is used to generate
    // a value y in [1,2], call Log2Estimate(y) and then add the exponent
    // for the power of two in the binary scientific representation of x.
    // For example a degree-3 estimate is
    //   float x;  // x > 0
    //   float result = Log2EstimateRR<float, 3>(x);
    template <typename T, size_t Degree>
    inline T Log2EstimateRR(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        int32_t p{};
        T y = std::frexp(x, &p);  // y in [1/2,1)
        y = static_cast<T>(2) * y;  // y in [1,2)
        --p;
        T poly = Log2Estimate<T, Degree>(y);
        T result = poly + static_cast<T>(p);
        return result;
    }

    template <typename T, size_t Degree>
    T constexpr GetLog2EstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(C_LOG2_EST_MAX_ERROR[Degree - 1]);
    }
}
