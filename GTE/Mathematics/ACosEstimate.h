// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// Approximations to acos(x) of the form f(x) = sqrt(1-x)*p(x), where the
// polynomial p(x) of degree D minimizes the quantity
// maximum{|acos(x)/sqrt(1-x) - p(x)| : x in [0,1]} over all polynomials of
// degree D.

#include <Mathematics/Constants.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    std::array<std::array<double, 9>, 8> constexpr C_ACOS_EST_COEFF =
    { {
        {   // degree 1
            +1.5707963267948966,
            -1.5658276442180141e-1
        },
        {   // degree 2
            +1.5707963267948966,
            -2.0347053865798365e-1,
            +4.6887774236182234e-2
        },
        {   // degree 3
            +1.5707963267948966,
            -2.1253291899190285e-1,
            +7.4773789639484223e-2,
            -1.8823635069382449e-2
        },
        {   // degree 4
            +1.5707963267948966,
            -2.1422258835275865e-1,
            +8.4936675142844198e-2,
            -3.5991475120957794e-2,
            +8.6946239090712751e-3
        },
        {   // degree 5
            +1.5707963267948966,
            -2.1453292139805524e-1,
            +8.7973089282889383e-2,
            -4.5130266382166440e-2,
            +1.9467466687281387e-2,
            -4.3601326117634898e-3
        },
        {   // degree 6
            +1.5707963267948966,
            -2.1458939285677325e-1,
            +8.8784960563641491e-2,
            -4.8887131453156485e-2,
            +2.7011519960012720e-2,
            -1.1210537323478320e-2,
            +2.3078166879102469e-3
        },
        {   // degree 7
            +1.5707963267948966,
            -2.1459960076929829e-1,
            +8.8986946573346160e-2,
            -5.0207843052845647e-2,
            +3.0961594977611639e-2,
            -1.7162031184398074e-2,
            +6.7072304676685235e-3,
            -1.2690614339589956e-3
        },
        {   // degree 8
            +1.5707963267948966,
            -2.1460143648688035e-1,
            +8.9034700107934128e-2,
            -5.0625279962389413e-2,
            +3.2683762943179318e-2,
            -2.0949278766238422e-2,
            +1.1272900916992512e-2,
            -4.1160981058965262e-3,
            +7.1796493341480527e-4
        }
    } };

    std::array<double, 8> constexpr C_ACOS_EST_MAX_ERROR =
    {
        9.0128265558585e-3,  // degree 1
        8.1851275863199e-4,  // degree 2
        8.8200141836526e-5,  // degree 3
        1.0563052499802e-5,  // degree 4
        1.3535063234649e-6,  // degree 5
        1.8169471727170e-7,  // degree 6
        2.5231622347022e-8,  // degree 7
        3.5952707477805e-9   // degree 8
    };
}

namespace gte
{
    // The input constraint is x in [0,1]. For example a degree-3 estimate is
    //   float x; // in [0,1]
    //   float result = ACosEstimate<float, 3>(x);
    template <typename T, size_t Degree>
    inline T ACosEstimate(T x)
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        size_t constexpr select = Degree - 1;
        auto constexpr& coeff = C_ACOS_EST_COEFF[select];
        size_t constexpr last = Degree;
        T poly = static_cast<T>(coeff[last]);
        for (size_t i = 0, index = last - 1; i < last; ++i, --index)
        {
            poly = static_cast<T>(coeff[index]) + poly * x;
        }
        poly = poly * std::sqrt(static_cast<T>(1) - x);
        return poly;
    }

    template <typename T, size_t Degree>
    T constexpr GetACosEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 8,
            "Invalid degree.");

        return static_cast<T>(C_ACOS_EST_MAX_ERROR[Degree - 1]);
    }
}
