// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

// The Chebyshev ratio is f(t,A) = sin(t*A)/sin(A) for t in [0,1] and A in
// [0,pi/2]. Let x = cos(A) and y = 1 - cos(A), both in [0,1]. As a function
// of y, a series representation for f(t,y) is
//   f(t,y) = sum_{i=0}^{infinity} c_{i}(t) y^{i}
// where c_0(t) = t, c_{i}(t) = c_{i-1}(t)*(i^2 - t^2)/(i*(2*i+1)) for i >= 1.
// The c_{i}(t) are polynomials in t of degree 2*i+1. The document
// https://www.geometrictools/com/Documentation/FastAndAccurateSlerp.pdf
// derives an approximation
//   g(t,y) = sum_{i=0}^{n-1} c_{i}(t) y^{i} + u_n c_{n}(t) y^n
// which has degree 2*n+1 in t and degree n in y. The constants u_n are chosen
// for balanced error bounds. ChebyshevRatioEstimate<T>::Degree implements
// this algorithm. If the angle A is restricted to [0,pi/4], then better
// estimates are obtained for the same computational cost. All that differs
// are the u_n-values. ChebyshevRatioEstimate<T>::DegreeR implements this
// algorithm. The functions return pairs {f(1-t,A), f(t,A)}, which is useful
// for spherical linear interpolation.

#include <Mathematics/Constants.h>
#include <array>
#include <cstddef>

namespace gte
{
    // Constants for ChebyshevRatio<T>::Degree.
    std::array<double, 16> constexpr C_CHBRAT_EST_U =
    {
        1.5149656562200644050,
        1.6410179946672027729,
        1.7124880779005808851,
        1.7593545031636841358,
        1.7927054757060019163,
        1.8177479632959470113,
        1.8372872973294931409,
        1.8529805143706497006,
        1.8658739107798316681,
        1.8766626700393858052,
        1.8858276947289707159,
        1.8937127486228939599,
        1.9005703533887863266,
        1.9065903281211855624,
        1.9119182032942771965,
        1.9166674811124804201
    };

    std::array<double, 16> constexpr C_CHBRAT_EST_MAX_ERROR =
    {
        1.8249897492955e-2,
        5.2760601519022e-3,
        1.8055057987877e-3,
        6.7244299646175e-4,
        2.6386437427495e-4,
        1.0731422197408e-4,
        4.4805894183764e-5,
        1.9088088593749e-5,
        8.2629028074211e-6,
        3.6237273527418e-6,
        1.6064797200289e-6,
        7.1872518425665e-7,
        3.2407757655229e-7,
        1.4712279927665e-7,
        6.7187475472075e-8,
        3.0844086507110e-8
    };

    template <typename T, size_t Degree>
    T constexpr C_CHBRAT_ACOEFF(size_t i)
    {
        return (Degree != (i + 1) ? static_cast<T>(1) : static_cast<T>(C_CHBRAT_EST_U[i]))
            / (static_cast<T>(i + 1) * static_cast<T>(2 * (i + 1) + 1));
    }

    template <typename T, size_t Degree>
    T constexpr C_CHBRAT_BCOEFF(size_t i)
    {
        return (Degree != (i + 1) ? static_cast<T>(1) : static_cast<T>(C_CHBRAT_EST_U[i]))
            * static_cast<T>(i + 1) / static_cast<T>(2 * (i + 1) + 1);
    }

    // Constants for ChebyshevRatio<T>::DegreeR.
    std::array<double, 12> constexpr C_CHBRAT_ESTR_U =
    {
        1.1021472152138613865,
        1.1239349540626744073,
        1.1351870374370363059,
        1.1421060160698368602,
        1.1468020192623136211,
        1.1502017494201659531,
        1.1527782928466798751,
        1.1547990001678465344,
        1.1564265502929687024,
        1.1577657226562501069,
        1.1588859375000000185,
        1.1598375000000000767
    };

    std::array<double, 12> constexpr C_CHBRAT_ESTR_MAX_ERROR =
    {
        8.6832275204274e-4,
        6.6040175097815e-5,
        6.1949661303018e-6,
        6.4578503422564e-7,
        7.1792162659179e-8,
        8.3364721792379e-9,
        9.9903230132981e-10,
        1.2262002524466e-10,
        1.5335510639148e-11,
        1.9472201628901e-12,
        2.5046631435544e-13,
        3.2751579226443e-14
    };

    template <typename T, size_t Degree>
    T constexpr C_CHBRAT_ACOEFF_R(size_t i)
    {
        return (Degree != (i + 1) ? static_cast<T>(1) : static_cast<T>(C_CHBRAT_ESTR_U[i]))
            / (static_cast<T>(i + 1)* static_cast<T>(2 * (i + 1) + 1));
    }

    template <typename T, size_t Degree>
    T constexpr C_CHBRAT_BCOEFF_R(size_t i)
    {
        return (Degree != (i + 1) ? static_cast<T>(1) : static_cast<T>(C_CHBRAT_ESTR_U[i]))
            * static_cast<T>(i + 1) / static_cast<T>(2 * (i + 1) + 1);
    }
}

namespace gte
{
    // Compute estimates for f(t,x) = sin(t*A)/sin(A), where t in [0,1],
    // A in [0,pi/2], x = cos(A) in [0,1], f0 is the estimate for f(1-t,x)
    // and f1 is the estimate for f(t,x). The approximating function is a
    // polynomial of two variables. The template parameter Degree must be in
    // {1..16}. The degree in t is 2*Degree+1 and the degree in x is Degree.
    template <typename T, size_t Degree>
    inline std::array<T, 2> ChebyshevRatioEstimate(T t, T x)
    {
        static_assert(
            1 <= Degree && Degree <= 16,
            "Invalid degree.");

        // An optimizing compiler will load only the required constants into
        // registers as literal values.
        std::array<T, 16> const a =
        {
            C_CHBRAT_ACOEFF<T, Degree>(0),
            C_CHBRAT_ACOEFF<T, Degree>(1),
            C_CHBRAT_ACOEFF<T, Degree>(2),
            C_CHBRAT_ACOEFF<T, Degree>(3),
            C_CHBRAT_ACOEFF<T, Degree>(4),
            C_CHBRAT_ACOEFF<T, Degree>(5),
            C_CHBRAT_ACOEFF<T, Degree>(6),
            C_CHBRAT_ACOEFF<T, Degree>(7),
            C_CHBRAT_ACOEFF<T, Degree>(8),
            C_CHBRAT_ACOEFF<T, Degree>(9),
            C_CHBRAT_ACOEFF<T, Degree>(10),
            C_CHBRAT_ACOEFF<T, Degree>(11),
            C_CHBRAT_ACOEFF<T, Degree>(12),
            C_CHBRAT_ACOEFF<T, Degree>(13),
            C_CHBRAT_ACOEFF<T, Degree>(14),
            C_CHBRAT_ACOEFF<T, Degree>(15)
        };

        std::array<T, 16> const b =
        {
            C_CHBRAT_BCOEFF<T, Degree>(0),
            C_CHBRAT_BCOEFF<T, Degree>(1),
            C_CHBRAT_BCOEFF<T, Degree>(2),
            C_CHBRAT_BCOEFF<T, Degree>(3),
            C_CHBRAT_BCOEFF<T, Degree>(4),
            C_CHBRAT_BCOEFF<T, Degree>(5),
            C_CHBRAT_BCOEFF<T, Degree>(6),
            C_CHBRAT_BCOEFF<T, Degree>(7),
            C_CHBRAT_BCOEFF<T, Degree>(8),
            C_CHBRAT_BCOEFF<T, Degree>(9),
            C_CHBRAT_BCOEFF<T, Degree>(10),
            C_CHBRAT_BCOEFF<T, Degree>(11),
            C_CHBRAT_BCOEFF<T, Degree>(12),
            C_CHBRAT_BCOEFF<T, Degree>(13),
            C_CHBRAT_BCOEFF<T, Degree>(14),
            C_CHBRAT_BCOEFF<T, Degree>(15)
        };

        T const one = static_cast<T>(1);
        T y = one - x;
        T term0 = one - t, term1 = t;
        T sqr0 = term0 * term0, sqr1 = term1 * term1;
        std::array<T, 2> f = { term0, term1 };
        for (size_t i = 0; i < Degree; ++i)
        {
            term0 *= (b[i] - a[i] * sqr0) * y;
            term1 *= (b[i] - a[i] * sqr1) * y;
            f[0] += term0;
            f[1] += term1;
        }
        return f;
    }

    template <typename T, size_t Degree>
    inline T GetChebyshevRatioEstimateMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 16,
            "Invalid degree.");

        return C_CHBRAT_EST_MAX_ERROR[Degree - 1];
    }

    // Compute estimates for f(t,x) = sin(t*A)/sin(A), where t in [0,1],
    // A in [0,pi/4], x = cos(A) in [0,1], f0 is the estimate for f(1-t,x)
    // and f1 is the estimate for f(t,x). The approximating function is a
    // polynomial of two variables. The template parameter Degree must be in
    // {1..12}. The degree in t is 2*Degree+1 and the degree in x is Degree.
    template <typename T, size_t Degree>
    inline std::array<T, 2> ChebyshevRatioEstimateR(T t, T x)
    {
        static_assert(
            1 <= Degree && Degree <= 12,
            "Invalid degree.");

        // An optimizing compiler will load only the required constants
        // into registers as literal values.
        std::array<T, 12> const a =
        {
            C_CHBRAT_ACOEFF_R<T, Degree>(0),
            C_CHBRAT_ACOEFF_R<T, Degree>(1),
            C_CHBRAT_ACOEFF_R<T, Degree>(2),
            C_CHBRAT_ACOEFF_R<T, Degree>(3),
            C_CHBRAT_ACOEFF_R<T, Degree>(4),
            C_CHBRAT_ACOEFF_R<T, Degree>(5),
            C_CHBRAT_ACOEFF_R<T, Degree>(6),
            C_CHBRAT_ACOEFF_R<T, Degree>(7),
            C_CHBRAT_ACOEFF_R<T, Degree>(8),
            C_CHBRAT_ACOEFF_R<T, Degree>(9),
            C_CHBRAT_ACOEFF_R<T, Degree>(10),
            C_CHBRAT_ACOEFF_R<T, Degree>(11)
        };

        std::array<T, 12> const b =
        {
            C_CHBRAT_BCOEFF_R<T, Degree>(0),
            C_CHBRAT_BCOEFF_R<T, Degree>(1),
            C_CHBRAT_BCOEFF_R<T, Degree>(2),
            C_CHBRAT_BCOEFF_R<T, Degree>(3),
            C_CHBRAT_BCOEFF_R<T, Degree>(4),
            C_CHBRAT_BCOEFF_R<T, Degree>(5),
            C_CHBRAT_BCOEFF_R<T, Degree>(6),
            C_CHBRAT_BCOEFF_R<T, Degree>(7),
            C_CHBRAT_BCOEFF_R<T, Degree>(8),
            C_CHBRAT_BCOEFF_R<T, Degree>(9),
            C_CHBRAT_BCOEFF_R<T, Degree>(10),
            C_CHBRAT_BCOEFF_R<T, Degree>(11)
        };

        T const one = static_cast<T>(1);
        T const y = one - x;
        T term0 = one - t, term1 = t;
        T sqr0 = term0 * term0, sqr1 = term1 * term1;
        std::array<T, 2> f = { term0, term1 };
        for (size_t i = 0; i < Degree; ++i)
        {
            term0 *= (b[i] - a[i] * sqr0) * y;
            term1 *= (b[i] - a[i] * sqr1) * y;
            f[0] += term0;
            f[1] += term1;
        }
        return f;
    }

    template <typename T, size_t Degree>
    T constexpr GetChebyshevRatioEstimateRMaxError()
    {
        static_assert(
            1 <= Degree && Degree <= 12,
            "Invalid degree.");

        return static_cast<T>(C_CHBRAT_ESTR_MAX_ERROR[Degree - 1]);
    }
}
