// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.11.20

#pragma once

// Compute the real-valued roots of a quartic polynomial with real-valued
// coefficients. The general quartic polynomial is
//   g(x) = g0 + g1 * x + g2 * x^2 + g3 * x^3 + g4 * x^4
// where g4 is not zero. The monic quartic polynomial is
//   m(x) = m0 + m1 * x + m2 * x^2 + m3 * x^3 + x^4
// The depressed quartic polynomial is
//   d(x) = d0 + d1 * x + d2 * x^2 + x^4
// The classification of roots and multiplicities is performed using rational
// arithmetic for exactness. For algorithmic details, see
// https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
// 
// The code uses bounding intervals for roots. For a polynomial of degree n
// with all real roots, Samuelson's inequality
// https://en.wikipedia.org/wiki/Samuelson%27s_inequality
// provides an interval [b0,b1] where
//   b0 = (-p[n-1] - (n-1) * s) / (n * p[n])
//   b1 = (-p[n-1] + (n-1) * s) / (n * p[n])
//   s = sqrt(p[n-1]^2 - 2 * n * p[n] * p[n-2] / (n-1))
// Applied to the general quartic (n - 4),
//   b0 = (-p3 - 3 * s) / (4 * p4)
//   b1 = (-p3 + 3 * s) / (4 * p4)
//   s = sqrt(p3^2 - 8 * p4 * p2 / 3)
// Applied to the depressed quartic when it has all real roots,
//   b0 = -sqrt(-3 * p2 / 2)
//   b1 = +sqrt(-3 * p2 / 2)
// 
// For a polynomial of degree n, Lagrange's bound is
//   b = max(1,|p[0]/p[n]|, |p[1]/p[n]|, ..., |p[n-1]/p[n]|)
// The real roots lie in the interval [-b,b].

#include <Mathematics/RootsCubic.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace gte
{
    template <typename T>
    class RootsQuartic
    {
    public:
        // Solve for the roots using a mixture of rational arithmetic and
        // floating-point arithmetic. The roots[] array must have at least 4
        // elements. The returned size_t is the number of valid roots in the
        // roots[] array.
        using Rational = BSRational<UIntegerAP32>;

        // Solve the general quartic g0 + g1*x + g2*x^2 + g3*x^3 + g4*x^4 = 0.
        static size_t Solve(bool useBisection,
            T const& g0, T const& g1, T const& g2, T const& g3, T const& g4,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test whether the degree is smaller than 4.
            T const zero = static_cast<T>(0);
            if (g4 == zero)
            {
                return RootsCubic<T>::Solve(useBisection, g0, g1, g2, g3, roots);
            }

            // Test for zero-valued roots.
            if (g0 == zero)
            {
                return HasZeroValuedRoots(useBisection, g1, g2, g3, g4, roots);
            }

            // At this time g0 and g4 are not zero. Transform the general
            // quartic to a depressed quartic, solve for its roots, and
            // inverse transform them to roots of the general quartic.
            Rational rD0{}, rD1{}, rD2{}, rM3Div4{};
            ComputeClassifiers(g0, g1, g2, g3, g4, rD0, rD1, rD2, rM3Div4);

            std::array<PolynomialRoot<Rational>, 4> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rD1, rD2, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM3Div4, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the monic quartic m0 + m1*x + m2*x^2 + m3*x^3 + x^4 = 0.
        static size_t Solve(bool useBisection,
            T const& m0, T const& m1, T const& m2, T const& m3,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test for zero-valued roots.
            T const zero = static_cast<T>(0);
            if (m0 == zero)
            {
                return HasZeroValuedRoots(useBisection, m1, m2, m3, roots);
            }

            // At this time m0 and m4 are not zero. Transform the general
            // quartic to a depressed quartic, solve for its roots, and
            // inverse transform them to roots of the general quartic.
            Rational rD0{}, rD1{}, rD2{}, rM3Div4{};
            ComputeClassifiers(m0, m1, m2, m3, rD0, rD1, rD2, rM3Div4);

            std::array<PolynomialRoot<Rational>, 4> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rD1, rD2, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM3Div4, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the depressed quartic d0 + d1*x + d2*x^2 + x^4 = 0.
        static size_t Solve(bool useBisection,
            T const& d0, T const& d1, T const& d2,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // The quartic is already depressed, so no transforming and
            // inverse transforming are necessary. A copy is required in case
            // T != Rational, in which case an implicit conversion occurs.
            std::array<PolynomialRoot<Rational>, 4> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, d0, d1, d2, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x, rRoots[i].m };
            }
            return numRoots;
        }

        static size_t ComputeDepressedRoots(bool useBisection,
            Rational const& rD0, Rational const& rD1, Rational const& rD2,
            PolynomialRoot<Rational>* rRoots)
        {
            if (useBisection)
            {
                return ComputeDepressedRootsBisection(rD0, rD1, rD2, rRoots);
            }
            else
            {
                return ComputeDepressedRootsClosedForm(rD0, rD1, rD2, rRoots);
            }
        }

    private:
        // Determine whether the polynomial has zero-valued roots.
        static size_t HasZeroValuedRoots(bool useBisection,
            T const& g1, T const& g2, T const& g3, T const& g4,
            PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (g1 == zero)
            {
                if (g2 == zero)
                {
                    if (g3 == zero)
                    {
                        roots[0] = { zero, 4 };
                        return 1;
                    }
                    else
                    {
                        size_t numRoots = RootsLinear<T>::Solve(g3, g4, roots);
                        roots[numRoots++] = { zero, 3 };
                        std::sort(roots, roots + numRoots);
                        return numRoots;
                    }
                }
                else
                {
                    size_t numRoots = RootsQuadratic<T>::Solve(useBisection, g2, g3, g4, roots);
                    roots[numRoots++] = { zero, 2 };
                    std::sort(roots, roots + numRoots);
                    return numRoots;
                }
            }
            else
            {
                size_t numRoots = RootsCubic<T>::Solve(useBisection, g1, g2, g3, g4, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        static size_t HasZeroValuedRoots(bool useBisection,
            T const& m1, T const& m2, T const& m3,
            PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (m1 == zero)
            {
                if (m2 == zero)
                {
                    if (m3 == zero)
                    {
                        roots[0] = { zero, 4 };
                        return 1;
                    }
                    else
                    {
                        size_t numRoots = RootsLinear<T>::Solve(m3, roots);
                        roots[numRoots++] = { zero, 3 };
                        std::sort(roots, roots + numRoots);
                        return numRoots;
                    }
                }
                else
                {
                    size_t numRoots = RootsQuadratic<T>::Solve(useBisection, m2, m3, roots);
                    roots[numRoots++] = { zero, 2 };
                    std::sort(roots, roots + numRoots);
                    return numRoots;
                }
            }
            else
            {
                size_t numRoots = RootsCubic<T>::Solve(useBisection, m1, m2, m3, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then g0, g1, g2, g3, and g4
        // are passed by reference to this function with no conversion. If T
        // is 'float' or 'double', the compiler generates implicit conversions
        // for g0, g1, g2, g3, and g4 to Rational values.
        static void ComputeClassifiers(
            Rational const& rG0, Rational const& rG1, Rational const& rG2, Rational const& rG3, Rational const& rG4,
            Rational& rD0, Rational& rD1, Rational& rD2, Rational& rM3Div4)
        {
            Rational rM0 = rG0 / rG4;
            Rational rM1 = rG1 / rG4;
            Rational rM2 = rG2 / rG4;
            Rational rM3 = rG3 / rG4;
            ComputeClassifiers(rM0, rM1, rM2, rM3, rD0, rD1, rD2, rM3Div4);
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then m0, m1, m2, and m3 are
        // passed by reference to this function with no conversion. If T is
        // 'float' or 'double', the compiler generates implicit conversions
        // for m0, m1, m2, and m3 to Rational values.
        static void ComputeClassifiers(
            Rational const& rM0, Rational const& rM1, Rational const& rM2, Rational const& rM3,
            Rational& rD0, Rational& rD1, Rational& rD2, Rational& rM3Div4)
        {
            rM3Div4 = Rational(1, 4) * rM3;
            Rational rM3Div4Sqr = rM3Div4 * rM3Div4;
            rD0 = rM0 - rM3Div4 * (rM1 - rM3Div4 * (rM2 - Rational(3) * rM3Div4Sqr));
            rD1 = rM1 - Rational(2) * rM3Div4 * (rM2 - Rational(4) * rM3Div4Sqr);
            rD2 = rM2 - Rational(6) * rM3Div4Sqr;
        }

        static size_t SolveBiquadratic(bool useBisection,
            Rational const& rD0, Rational const& rD2,
            PolynomialRoot<Rational>* rRoots)
        {
            Rational rS = Rational(-0.5) * rD2;
            Rational rT = rS * rS - rD0;
            int32_t signT = rT.GetSign();
            if (signT > 0)
            {
                std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                RootsQuadratic<Rational>::ComputeDepressedRoots(useBisection, -rT, rQRoots.data());
                Rational rSqrtT = rQRoots[1].x;
                Rational rSPsqrtT = rS + rSqrtT;
                Rational rSMsqrtT = rD0 / rSPsqrtT;
                int32_t signSPsqrtT = rSPsqrtT.GetSign();
                int32_t signSMsqrtT = rSMsqrtT.GetSign();
                if (signSMsqrtT > 0)
                {
                    // Four real roots.
                    RootsQuadratic<Rational>::ComputeDepressedRoots(useBisection, -rSMsqrtT, rQRoots.data());
                    Rational r0 = rQRoots[1].x;
                    RootsQuadratic<Rational>::ComputeDepressedRoots(useBisection, -rSPsqrtT, rQRoots.data());
                    Rational r1 = rQRoots[1].x;
                    rRoots[0] = { r0, 1 };
                    rRoots[1] = { -r0, 1 };
                    rRoots[2] = { r1, 1 };
                    rRoots[3] = { -r1, 1 };
                    std::sort(rRoots, rRoots + 4);
                    return 4;
                }
                else if (signSPsqrtT < 0)
                {
                    // Two complex-conjugate pairs.
                    return 0;
                }
                else  // signSMsqrtT < 0 and signSPsqrtT > 0
                {
                    // Two real roots, one complex-conjugate pair.
                    RootsQuadratic<Rational>::ComputeDepressedRoots(useBisection, -rSPsqrtT, rQRoots.data());
                    Rational r0 = rQRoots[1].x;
                    if (r0.GetSign() > 0)
                    {
                        rRoots[0] = { -r0, 1 };
                        rRoots[1] = { r0, 1 };
                    }
                    else
                    {
                        rRoots[0] = { r0, 1 };
                        rRoots[1] = { -r0, 1 };
                    }
                    return 2;
                }
            }
            else if (signT < 0)
            {
                // Two complex-conjugate pairs.
                return 0;
            }
            else
            {
                if (rS.GetSign() > 0)
                {
                    // Two real roots, each of multiplicity 2.
                    std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                    RootsQuadratic<Rational>::ComputeDepressedRoots(useBisection, -rS, rQRoots.data());
                    Rational r0 = rQRoots[1].x;
                    if (r0.GetSign() > 0)
                    {
                        rRoots[0] = { -r0, 2 };
                        rRoots[1] = { r0, 2 };
                    }
                    else
                    {
                        rRoots[0] = { r0, 2 };
                        rRoots[1] = { -r0, 2 };
                    }
                    return 2;
                }
                else
                {
                    // One complex-conjugate pair of multiplicity 2.
                    return 0;
                }
           }
       }

        static size_t ComputeDepressedRootsBisection(
            Rational const& rD0, Rational const& rD1, Rational const& rD2,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            int32_t signD1 = rD1.GetSign();
            int32_t signD2 = rD2.GetSign();
            if (signD0 == 0)
            {
                if (signD1 == 0)
                {
                    if (signD2 > 0)
                    {
                        // One real root, multiplicity 2.
                        rRoots[0] = { Rational(0), 2 };
                        return 1;
                    }
                    else if (signD2 < 0)
                    {
                        // Three real roots, one with multiplicity 2, two with
                        // multiplicity 1.
                        std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                        RootsQuadratic<Rational>::ComputeDepressedRoots(true, rD2, rQRoots.data());
                        Rational rSqrtNegD2 = rQRoots[1].x;
                        rRoots[0] = { -rSqrtNegD2, 1 };
                        rRoots[1] = { Rational(0), 2 };
                        rRoots[2] = { rSqrtNegD2, 1 };
                        return 3;
                    }
                    else
                    {
                        // One real root, multiplicity 4.
                        rRoots[0] = { Rational(0), 4 };
                        return 1;
                    }
                }
                else
                {
                    // Zero is a root of multiplicity 1. The cubic solver
                    // computes the other roots.
                    size_t numRoots = RootsCubic<Rational>::ComputeDepressedRoots(true, rD1, rD2, rRoots);
                    rRoots[numRoots++] = { Rational(0), 1 };
                    std::sort(rRoots, rRoots + numRoots);
                    return numRoots;
                }
            }

            // At this time d0 != 0.
            if (signD1 == 0)
            {
                return SolveBiquadratic(true, rD0, rD2, rRoots);
            }

            // At this time, d0 != 0 and d != 0.
            Rational rD0sqr = rD0 * rD0, rD1sqr = rD1 * rD1, rD2sqr = rD2 * rD2;
            Rational rDelta = rD1sqr * (Rational(-27) * rD1sqr +
                Rational(4) * rD2 * (Rational(36) * rD0 - rD2sqr)) +
                Rational(16) * rD0 * (rD2sqr * (rD2sqr - Rational(8) * rD0) +
                    Rational(16) * rD0sqr);

            int32_t signDelta = rDelta.GetSign();
            if (signDelta == 0)
            {
                // Process the repeated roots.
                Rational rA0 = Rational(12) * rD0 + rD2sqr;
                if (rA0.GetSign() == 0)
                {
                    // Case (x-r0)^3 (x-r1), d2 < 0 guaranteed.
                    Rational r0 = Rational(-0.75) * rD1 / rD2;
                    Rational r1 = Rational(-3) * r0;
                    if (r0 < r1)
                    {
                        rRoots[0] = { r0, 3 };
                        rRoots[1] = { r1, 1 };
                    }
                    else
                    {
                        rRoots[0] = { r1, 1 };
                        rRoots[1] = { r0, 3 };
                    }
                    return 2;
                }

                // Non-zero denominator guaranteed.
                Rational rA1 = Rational(4) * rD0 - rD2sqr;
                Rational r0 = -rD1 * rA0 / (Rational(9) * rD1sqr - Rational(2) * rD2 * rA1);
                rRoots[0] = { r0, 2 };
                Rational rQDiscriminant = -(rD2 + Rational(2) * r0 * r0);
                if (rQDiscriminant.GetSign() > 0)
                {
                    // Case (x-r0)^2 (x-r1) (x-r2).
                    std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                    RootsQuadratic<Rational>::ComputeDepressedRoots(true, -rQDiscriminant, rQRoots.data());
                    Rational rSqrtQDiscriminant = rQRoots[1].x;
                    Rational r1 = -r0 - rSqrtQDiscriminant;
                    Rational r2 = -r0 + rSqrtQDiscriminant;
                    rRoots[1] = { r1, 1 };
                    rRoots[2] = { r2, 1 };
                    std::sort(rRoots, rRoots + 3);
                    return 3;
                }

                // Case (x-r0)^2 (x-z0) (x-z0c).
                return 1;
            }

            if (signDelta > 0 && rD2.GetSign() > 0)
            {
                // Two complex-conjugate pairs.
                return 0;
            }

            // Transform the discriminant (monic cubic) to a depressed cubic.
            Rational rM0 = Rational(0.125) * (Rational(4) * rD0 * rD2 - rD1sqr);
            Rational rM1 = -rD0;
            Rational rM2 = Rational(-0.5) * rD2;
            Rational rM2Div3 = Rational(1, 3) * rM2;
            Rational rC0 = rM0 - rM2Div3 * (rM1 - Rational(2) * rM2Div3 * rM2Div3);
            Rational rC1 = rM1 - rM2 * rM2Div3;

            // Compute the roots of the depressed cubic. The minimum root of
            // the depressed polynomial corresponds to the maximum root of the
            // monic polynomial. Also inverse-transform the root.
            std::array<PolynomialRoot<Rational>, 3> rCRoots{};
            RootsCubic<Rational>::ComputeDepressedRoots(true, rC0, rC1, rCRoots.data());
            Rational rT = rCRoots[0].x - rM2Div3;

            std::array<PolynomialRoot<Rational>, 2> rQRoots{};
            Rational rAlphaSqr = Rational(2) * rT - rD2;
            RootsQuadratic<Rational>::ComputeDepressedRoots(true, -rAlphaSqr, rQRoots.data());
            Rational rAlpha = rQRoots[1].x;
            Rational rSignD1 = (rD1.GetSign() > 0 ? Rational(1) : Rational(-1));
            Rational rArg = rT * rT - rD0;
            RootsQuadratic<Rational>::ComputeDepressedRoots(true, -rArg, rQRoots.data());
            Rational rBeta = rSignD1 * rQRoots[1].x;
            Rational rDiscr0 = rAlphaSqr - Rational(4) * (rT + rBeta);
            RootsQuadratic<Rational>::ComputeDepressedRoots(true, -rDiscr0, rQRoots.data());
            Rational rSqrtDiscr0 = rQRoots[1].x;
            Rational rDiscr1 = rAlphaSqr - Rational(4) * (rT - rBeta);
            RootsQuadratic<Rational>::ComputeDepressedRoots(true, -rDiscr1, rQRoots.data());
            Rational rSqrtDiscr1 = rQRoots[1].x;

            Rational const half(0.5);
            if (signDelta > 0)
            {
                // Case (x-r0)(x-r1)(x-r2)(x-r3).
                Rational r0 = half * (rAlpha - rSqrtDiscr0);
                Rational r1 = half * (rAlpha + rSqrtDiscr0);
                Rational r2 = half * (-rAlpha - rSqrtDiscr1);
                Rational r3 = half * (-rAlpha + rSqrtDiscr1);
                rRoots[0] = { r0, 1 };
                rRoots[1] = { r1, 1 };
                rRoots[2] = { r2, 1 };
                rRoots[3] = { r3, 1 };
                std::sort(rRoots, rRoots + 4);
                return 4;
            }
            else // signDelta < 0
            {
                // Case (x-r0)(x-r1)(x-z0)(x-z0c).
                Rational r0{}, r1{};
                if (signD1 > 0)
                {
                    r0 = half * (-rAlpha - rSqrtDiscr1);
                    r1 = half * (-rAlpha + rSqrtDiscr1);
                }
                else
                {
                    r0 = half * (rAlpha - rSqrtDiscr0);
                    r1 = half * (rAlpha + rSqrtDiscr0);
                }
                rRoots[0] = { r0, 1 };
                rRoots[1] = { r1, 1 };
                return 2;
            }
        }

        static size_t ComputeDepressedRootsClosedForm(
            Rational const& rD0, Rational const& rD1, Rational const& rD2,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            int32_t signD1 = rD1.GetSign();
            int32_t signD2 = rD2.GetSign();
            if (signD0 == 0)
            {
                if (signD1 == 0)
                {
                    if (signD2 > 0)
                    {
                        // One real root, multiplicity 2.
                        rRoots[0] = { Rational(0), 2 };
                        return 1;
                    }
                    else if (signD2 < 0)
                    {
                        // Three real roots, one with multiplicity 2, two with
                        // multiplicity 1.
                        std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                        RootsQuadratic<Rational>::ComputeDepressedRoots(false, rD2, rQRoots.data());
                        Rational rSqrtNegD2 = rQRoots[1].x;
                        rRoots[0] = { -rSqrtNegD2, 1 };
                        rRoots[1] = { Rational(0), 2 };
                        rRoots[2] = { rSqrtNegD2, 1 };
                        return 3;
                    }
                    else
                    {
                        // One real root, multiplicity 4.
                        rRoots[0] = { Rational(0), 4 };
                        return 1;
                    }
                }
                else
                {
                    // Zero is a root of multiplicity 1. The cubic solver
                    // computes the other roots.
                    size_t numRoots = RootsCubic<Rational>::ComputeDepressedRoots(false, rD1, rD2, rRoots);
                    rRoots[numRoots++] = { Rational(0), 1 };
                    std::sort(rRoots, rRoots + numRoots);
                    return numRoots;
                }
            }

            // At this time d0 != 0.
            if (signD1 == 0)
            {
                return SolveBiquadratic(false, rD0, rD2, rRoots);
            }

            // At this time, d0 != 0 and d != 0.
            Rational rD0sqr = rD0 * rD0, rD1sqr = rD1 * rD1, rD2sqr = rD2 * rD2;
            Rational rDelta = rD1sqr * (Rational(-27) * rD1sqr +
                Rational(4) * rD2 * (Rational(36) * rD0 - rD2sqr)) +
                Rational(16) * rD0 * (rD2sqr * (rD2sqr - Rational(8) * rD0) +
                    Rational(16) * rD0sqr);

            int32_t signDelta = rDelta.GetSign();
            if (signDelta == 0)
            {
                // Process the repeated roots.
                Rational rA0 = Rational(12) * rD0 + rD2sqr;
                if (rA0.GetSign() == 0)
                {
                    // Case (x-r0)^3 (x-r1), d2 < 0 guaranteed.
                    Rational r0 = Rational(-0.75) * rD1 / rD2;
                    Rational r1 = Rational(-3) * r0;
                    if (r0 < r1)
                    {
                        rRoots[0] = { r0, 3 };
                        rRoots[1] = { r1, 1 };
                    }
                    else
                    {
                        rRoots[0] = { r1, 1 };
                        rRoots[1] = { r0, 3 };
                    }
                    return 2;
                }

                // Non-zero denominator guaranteed.
                Rational rA1 = Rational(4) * rD0 - rD2sqr;
                Rational r0 = -rD1 * rA0 / (Rational(9) * rD1sqr - Rational(2) * rD2 * rA1);
                rRoots[0] = { r0, 2 };
                Rational rQDiscriminant = -(rD2 + Rational(2) * r0 * r0);
                if (rQDiscriminant.GetSign() > 0)
                {
                    // Case (x-r0)^2 (x-r1) (x-r2).
                    std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                    RootsQuadratic<Rational>::ComputeDepressedRoots(false, -rQDiscriminant, rQRoots.data());
                    Rational rSqrtQDiscriminant = rQRoots[1].x;
                    Rational r1 = -r0 - rSqrtQDiscriminant;
                    Rational r2 = -r0 + rSqrtQDiscriminant;
                    rRoots[1] = { r1, 1 };
                    rRoots[2] = { r2, 1 };
                    std::sort(rRoots, rRoots + 3);
                    return 3;
                }

                // Case (x-r0)^2 (x-z0) (x-z0c).
                return 1;
            }

            if (signDelta > 0 && rD2.GetSign() > 0)
            {
                // Two complex-conjugate pairs.
                return 0;
            }

            // Transform the discriminant (monic cubic) to a depressed cubic.
            Rational rM0 = Rational(0.125) * (Rational(4) * rD0 * rD2 - rD1sqr);
            Rational rM1 = -rD0;
            Rational rM2 = Rational(-0.5) * rD2;
            Rational rM2Div3 = Rational(1, 3) * rM2;
            Rational rC0 = rM0 - rM2Div3 * (rM1 - Rational(2) * rM2Div3 * rM2Div3);
            Rational rC1 = rM1 - rM2 * rM2Div3;

            // Compute the roots of the depressed cubic. The minimum root of
            // the depressed polynomial corresponds to the maximum root of the
            // monic polynomial. Also inverse-transform the root.
            std::array<PolynomialRoot<Rational>, 3> rCRoots{};
            RootsCubic<Rational>::ComputeDepressedRoots(false, rC0, rC1, rCRoots.data());
            Rational rT = rCRoots[0].x - rM2Div3;

            std::array<PolynomialRoot<Rational>, 2> rQRoots{};
            Rational rAlphaSqr = Rational(2) * rT - rD2;
            RootsQuadratic<Rational>::ComputeDepressedRoots(false, -rAlphaSqr, rQRoots.data());
            Rational rAlpha = rQRoots[1].x;
            Rational rSignD1 = (rD1.GetSign() > 0 ? Rational(1) : Rational(-1));
            Rational rArg = rT * rT - rD0;
            RootsQuadratic<Rational>::ComputeDepressedRoots(false, -rArg, rQRoots.data());
            Rational rBeta = rSignD1 * rQRoots[1].x;
            Rational rDiscr0 = rAlphaSqr - Rational(4) * (rT + rBeta);
            RootsQuadratic<Rational>::ComputeDepressedRoots(false, -rDiscr0, rQRoots.data());
            Rational rSqrtDiscr0 = rQRoots[1].x;
            Rational rDiscr1 = rAlphaSqr - Rational(4) * (rT - rBeta);
            RootsQuadratic<Rational>::ComputeDepressedRoots(false, -rDiscr1, rQRoots.data());
            Rational rSqrtDiscr1 = rQRoots[1].x;

            Rational const half(0.5);
            if (signDelta > 0)
            {
                // Case (x-r0)(x-r1)(x-r2)(x-r3).
                Rational r0 = half * (rAlpha - rSqrtDiscr0);
                Rational r1 = half * (rAlpha + rSqrtDiscr0);
                Rational r2 = half * (-rAlpha - rSqrtDiscr1);
                Rational r3 = half * (-rAlpha + rSqrtDiscr1);
                rRoots[0] = { r0, 1 };
                rRoots[1] = { r1, 1 };
                rRoots[2] = { r2, 1 };
                rRoots[3] = { r3, 1 };
                std::sort(rRoots, rRoots + 4);
                return 4;
            }
            else // signDelta < 0
            {
                // Case (x-r0)(x-r1)(x-z0)(x-z0c).
                Rational r0{}, r1{};
                if (signD1 > 0)
                {
                    r0 = half * (-rAlpha - rSqrtDiscr1);
                    r1 = half * (-rAlpha + rSqrtDiscr1);
                }
                else
                {
                    r0 = half * (rAlpha - rSqrtDiscr0);
                    r1 = half * (rAlpha + rSqrtDiscr0);
                }
                rRoots[0] = { r0, 1 };
                rRoots[1] = { r1, 1 };
                return 2;
            }
        }
    };
}
