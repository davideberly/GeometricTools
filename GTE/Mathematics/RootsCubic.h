// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.11.20

#pragma once

// Compute the real-valued roots of a cubic polynomial with real-valued
// coefficients. The general cubic polynomial is
//   g(x) = g0 + g1 * x + g2 * x^2 + g3 * x^3
// where g3 is not zero. The monic cubic polynomial is
//   m(x) = m0 + m1 * x + m2 * x^2 + x^3
// The depressed cubic polynomial is
//   d(x) = d0 + d1 * x + x^3
// The classification of roots and multiplicities is performed using rational
// arithmetic for exactness. For algorithmic details, see
// https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
//
// The code uses bisection on bounding intervals for roots. For a polynomial
// of degree n with all real roots, Samuelson's inequality
// https://en.wikipedia.org/wiki/Samuelson%27s_inequality
// provides a bounding interval [b0,b1] where
//   b0 = (-p[n-1] - (n-1) * s) / (n * p[n])
//   b1 = (-p[n-1] + (n-1) * s) / (n * p[n])
//   s = sqrt(p[n-1]^2 - 2 * n * p[n] * p[n-2] / (n-1))
// Applied to the general cubic,
//   b0 = (-p2 - 2 * s) / (3 * p3)
//   b1 = (-p2 + 2 * s) / (3 * p3)
//   s = sqrt(p2^2 - 3 * p3 * p1)
// Applied to the depressed cubic,
//   b0 = -sqrt(-4 * p1 / 3)
//   b1 = +sqrt(-4 * p1 / 3)
// 
// For a polynomial of degree n, Lagrange's bound is
//   b = max(1,|p[0]/p[n]|, |p[1]/p[n]|, ..., |p[n-1]/p[n]|)
// The real roots lie in the interval [-b,b].

#include <Mathematics/RootsQuadratic.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace gte
{
    template <typename T>
    class RootsCubic
    {
    public:
        // Solve for the roots using a mixture of rational arithmetic and
        // floating-point arithmetic. The roots[] array must have at least 3
        // elements. The returned size_t is the number of valid roots in the
        // roots[] array.
        using Rational = BSRational<UIntegerAP32>;

        // Solve the general cubic g0 + g1*x + g2*x^2 + g3*x^3 = 0.
        static size_t Solve(bool useBisection,
            T const& g0, T const& g1, T const& g2, T const& g3,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test whether the degree is smaller than 3.
            T const zero = static_cast<T>(0);
            if (g3 == zero)
            {
                return RootsQuadratic<T>::Solve(useBisection, g0, g1, g2, roots);
            }

            // Test for zero-valued roots.
            if (g0 == zero)
            {
                return HasZeroValuedRoots(useBisection, g1, g2, g3, roots);
            }

            // At this time g0 and g3 are not zero. Transform the general
            // cubic to a depressed cubic, solve for its roots, and inverse
            // transform them to roots of the general cubic.
            Rational rD0{}, rD1{}, rM2Div3{};
            ComputeClassifiers(g0, g1, g2, g3, rD0, rD1, rM2Div3);

            std::array<PolynomialRoot<Rational>, 3> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rD1, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM2Div3, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the monic cubic m0 + m1*x + m2*x^2 + x^3 = 0.
        static size_t Solve(bool useBisection,
            T const& m0, T const& m1, T const& m2,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test for zero-valued roots.
            T const zero = static_cast<T>(0);
            if (m0 == zero)
            {
                return HasZeroValuedRoots(useBisection, m1, m2, roots);
            }

            // At this time p0 and p3 are not zero. Transform the monic cubic
            // to a depressed cubic, solve for its roots, and inverse
            // transform them to roots of the monic cubic.
            Rational rD0{}, rD1{}, rM2Div3{};
            ComputeClassifiers(m0, m1, m2, rD0, rD1, rM2Div3);

            std::array<PolynomialRoot<Rational>, 3> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rD1, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM2Div3, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the depressed cubic d0 + d1 * x + x^3 = 0.
        static size_t Solve(bool useBisection,
            T const& d0, T const& d1,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // The cubic is already depressed, so no transforming and inverse
            // transforming are necessary. A copy is required in case
            // T != Rational, in which case an implicit conversion occurs.
            std::array<PolynomialRoot<Rational>, 3> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, d0, d1, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x, rRoots[i].m };
            }
            return numRoots;
        }

        static size_t ComputeDepressedRoots(bool useBisection,
            Rational const& rD0, Rational const& rD1,
            PolynomialRoot<Rational>* rRoots)
        {
            if (useBisection)
            {
                return ComputeDepressedRootsBisection(rD0, rD1, rRoots);
            }
            else
            {
                return ComputeDepressedRootsClosedForm(rD0, rD1, rRoots);
            }
        }

    private:
        // Determine whether the polynomial has zero-valued roots.
        static size_t HasZeroValuedRoots(bool useBisection,
            T const& g1, T const& g2, T const& g3,
            PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (g1 == zero)
            {
                if (g2 == zero)
                {
                    roots[0] = { zero, 3 };
                    return 1;
                }
                else
                {
                    size_t numRoots = RootsLinear<T>::Solve(g2, g3, roots);
                    roots[numRoots++] = { zero, 2 };
                    std::sort(roots, roots + numRoots);
                    return numRoots;
                }
            }
            else
            {
                size_t numRoots = RootsQuadratic<T>::Solve(useBisection, g1, g2, g3, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        static size_t HasZeroValuedRoots(bool useBisection,
            T const& m1, T const& m2,
            PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (m1 == zero)
            {
                if (m2 == zero)
                {
                    roots[0] = { zero, 3 };
                    return 1;
                }
                else
                {
                    size_t numRoots = RootsLinear<T>::Solve(m2, roots);
                    roots[numRoots++] = { zero, 2 };
                    std::sort(roots, roots + numRoots);
                    return numRoots;
                }
            }
            else
            {
                size_t numRoots = RootsQuadratic<T>::Solve(useBisection, m1, m2, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then g0, g1, g2, and g3 are
        // passed by reference to this function with no conversion. If T is
        // 'float' or 'double', the compiler generates implicit conversions
        // for g0, g1, g2, and g3 to Rational values.
        static void ComputeClassifiers(
            Rational const& rG0, Rational const& rG1, Rational const& rG2, Rational const& rG3,
            Rational& rD0, Rational& rD1, Rational& rM2Div3)
        {
            Rational rM0 = rG0 / rG3;
            Rational rM1 = rG1 / rG3;
            Rational rM2 = rG2 / rG3;
            ComputeClassifiers(rM0, rM1, rM2, rD0, rD1, rM2Div3);
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then g0, g1, and g2 are
        // passed by reference to this function with no conversion. If T is
        // 'float' or 'double', the compiler generates implicit conversions
        // for g0, g1, and g2 to Rational values.
        static void ComputeClassifiers(
            Rational const& rM0, Rational const& rM1, Rational const& rM2,
            Rational& rD0, Rational& rD1, Rational& rM2Div3)
        {
            rM2Div3 = Rational(1, 3) * rM2;
            rD0 = rM0 - rM2Div3 * (rM1 - Rational(2) * rM2Div3 * rM2Div3);
            rD1 = rM1 - rM2 * rM2Div3;
        }

        static size_t ComputeDepressedRootsBisection(
            Rational const& rD0, Rational const& rD1,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            int32_t signD1 = rD1.GetSign();
            if (signD0 == 0)
            {
                if (signD1 > 0)
                {
                    // One real root, multiplicity 1.
                    rRoots[0] = { Rational(0), 1 };
                    return 1;
                }
                else if (signD1 < 0)
                {
                    // Three real roots, each multiplicity 1.
                    size_t numRoots = RootsQuadratic<T>::ComputeDepressedRoots(
                        true, rD1, rRoots);
                    rRoots[numRoots++] = { Rational(0), 1 };
                    std::sort(rRoots, rRoots + numRoots);
                    return numRoots;
                }
                else  // signD1 = 0
                {
                    // One real root, multiplicity 3.
                    rRoots[0] = { Rational(0), 3 };
                    return 1;
                }
            }

            if (signD1 == 0) // and d0 != 0
            {
                // One real root, multiplicity 1.
                double d0 = rD0;
                double b = std::max(1.0, std::fabs(d0));
                auto F = [&d0](double x)
                {
                    return gte::FMA(x, x * x, d0);
                };

                // Bisect on the interval [-b,b].
                Rational const half = static_cast<Rational>(0.5);
                double xMin = -b;
                double xMax = b;
                PolynomialRootBisect<double>(F, -1, +1, xMin, xMax);
                rRoots[0] = { half * (Rational(xMin) + Rational(xMax)), 1 };
                return 1;
            }

            Rational rDelta = Rational(-27) * rD0 * rD0 + Rational(-4) * rD1 * rD1 * rD1;
            int32_t signDelta = rDelta.GetSign();
            if (signDelta > 0)
            {
                // Three real roots, each multiplicity 1. The derivative of
                // F(x) = x^3 + d1 * x + d0 is F'(x) = 3 * x^2 + d1 and must
                // have two real roots x0 and x1, which means d1 < 0. Let
                // s = sqrt(-d1 / 3). The F'(x) roots are x0 = -s and x1 = s.
                // Using Samuelson's inequality, an interval bounding the
                // roots is [-2 * s, 2 * s]. Partition the interval into
                // [-2 * s, -s], [-s, s], and [s, 2 * s]. Use bisection on
                // each interval to estimate the roots of F(x).
                std::array<PolynomialRoot<Rational>, 2> rQRoots{};
                RootsQuadratic<T>::ComputeDepressedRoots(
                    true, Rational(1, 3) * rD1, rQRoots.data());
                Rational rS = rQRoots[1].x;
                Rational rTwoS = Rational(2) * rS;
                double d0 = rD0;
                double d1 = rD1;
                double s = rS;
                double twoS = rTwoS;
                auto F = [&d0, &d1](double x)
                {
                    return gte::FMA(x, gte::FMA(x, x, d1), d0);
                };

                Rational const half = static_cast<Rational>(0.5);
                double xMin{}, xMax{};

                // Bisect on the interval [-2 * s, s].
                xMin = -twoS;
                xMax = -s;
                PolynomialRootBisect<double>(F, -1, +1, xMin, xMax);
                rRoots[0] = { half * (Rational(xMin) + Rational(xMax)), 1 };

                // Bisect on the interval [-2 * s, s].
                xMin = -s;
                xMax = s;
                PolynomialRootBisect<double>(F, +1, -1, xMin, xMax);
                rRoots[1] = { half * (Rational(xMin) + Rational(xMax)), 1 };

                // Bisect on the interval [s, 2 * s].
                xMin = s;
                xMax = twoS;
                PolynomialRootBisect<double>(F, -1, +1, xMin, xMax);
                rRoots[2] = { half * (Rational(xMin) + Rational(xMax)), 1 };
                return 3;
            }
            else if (signDelta < 0)
            {
                // One real root, multiplicity 1. The Cauchy bound for F(x) is
                // b = max{1,|d0|,|d1|}. Use bisection on the interval [-b,b]
                // to estimate the root.
                double d0 = rD0;
                double d1 = rD1;
                double b = std::max(1.0, std::max(std::fabs(d0), std::fabs(d1)));
                auto F = [&d0, &d1](double x)
                {
                    return gte::FMA(x, gte::FMA(x, x, d1), d0);
                };

                // Bisect on the interval [-b,b].
                Rational const half = static_cast<Rational>(0.5);
                double xMin = -b;
                double xMax = b;
                PolynomialRootBisect<double>(F, -1, +1, xMin, xMax);
                rRoots[0] = { half * (Rational(xMin) + Rational(xMax)), 1 };
                return 1;
            }
            else  // delta = 0
            {
                // One real root, multiplicity 1. One real root, multiplicity
                // 2. The roots are rational numbers, so F(x) = 0 exactly for
                // each root x.
                Rational rX0 = Rational(-3, 2) * rD0 / rD1;
                Rational rX1 = Rational(-2) * rX0;
                if (rX0 < rX1)
                {
                    rRoots[0] = { rX0, 2 };
                    rRoots[1] = { rX1, 1 };
                }
                else
                {
                    rRoots[0] = { rX1, 1 };
                    rRoots[1] = { rX0, 2 };
                }
                return 2;
            }
        }

        static size_t ComputeDepressedRootsClosedForm(
            Rational const& rD0, Rational const& rD1,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            int32_t signD1 = rD1.GetSign();
            if (signD0 == 0)
            {
                if (signD1 > 0)
                {
                    // One real root, multiplicity 1.
                    rRoots[0] = { Rational(0), 1 };
                    return 1;
                }
                else if (signD1 < 0)
                {
                    // Three real roots, each multiplicity 1.
                    Rational rSqrtNegD1 = std::sqrt(-rD1);
                    rRoots[2] = { rSqrtNegD1, 1 };
                    rRoots[1] = { Rational(0), 1 };
                    rSqrtNegD1.Negate();
                    rRoots[0] = { rSqrtNegD1, 1 };
                    return 3;
                }
                else  // signD1 = 0
                {
                    // One real root, multiplicity 3.
                    rRoots[0] = { Rational(0), 3 };
                    return 1;
                }
            }

            if (signD1 == 0) // and d0 != 0
            {
                // One real root, multiplicity 1.
                Rational const r1Div3(1, 3);
                if (rD0.GetSign() > 0)
                {
                    rRoots[0] = { -std::pow(rD0, r1Div3), 1 };
                }
                else
                {
                    rRoots[0] = { std::pow(-rD0, r1Div3), 1 };
                }
                return 1;
            }

            Rational rDelta = Rational(-27) * rD0 * rD0 + Rational(-4) * rD1 * rD1 * rD1;
            int32_t signDelta = rDelta.GetSign();
            if (signDelta > 0)
            {
                // Three real roots, each multiplicity 1.
                Rational const rSqrt3 = std::sqrt(3.0), r3Div2(3, 2), r1Div3(1, 3);
                Rational rD1Div3 = rD1 * r1Div3;
                Rational rRho = std::pow(std::fabs(rD1Div3), r3Div2);
                Rational rCbrtRho = std::pow(rRho, r1Div3);
                Rational rTheta = std::atan2(std::sqrt(rDelta / Rational(27)), -rD0);
                Rational rThetaDiv3 = rTheta * r1Div3;
                Rational rCosThetaDiv3 = std::cos(rThetaDiv3);
                Rational rSinThetaDiv3 = std::sin(rThetaDiv3);
                Rational rTemp0 = rCbrtRho * rCosThetaDiv3;
                Rational rTemp1 = rSqrt3 * rCbrtRho * rSinThetaDiv3;
                Rational r0 = Rational(2) * rTemp0;
                Rational r1 = -rTemp0 - rTemp1;
                Rational r2 = -rTemp0 + rTemp1;
                if (rSinThetaDiv3.GetSign() > 0)
                {
                    rRoots[0] = { r1, 1 };
                    rRoots[1] = { r2, 1 };
                    rRoots[2] = { r0, 1 };
                }
                else
                {
                    rRoots[0] = { r2, 1 };
                    rRoots[1] = { r1, 1 };
                    rRoots[2] = { r0, 1 };
                }
                return 3;
            }
            else if (signDelta < 0)
            {
                // One real root, multiplicity 1.
                Rational const r1Div3(1, 3);
                Rational rSqrtNegDeltaDiv27 = std::sqrt(-rDelta / Rational(27));
                Rational rD1Div3 = rD1 * r1Div3;
                if (signD0 < 0)
                {
                    Rational rW = Rational(1, 2) * (-rD0 + rSqrtNegDeltaDiv27);
                    Rational rCbrtW = std::pow(rW, r1Div3);
                    Rational r0 = rCbrtW - rD1Div3 / rCbrtW;
                    rRoots[0] = { r0, 1 };
                }
                else
                {
                    Rational rNegY = Rational(1, 2) * (rD0 + rSqrtNegDeltaDiv27);
                    Rational rCbrtY = -std::pow(rNegY, r1Div3);
                    Rational r0 = rCbrtY - rD1Div3 / rCbrtY;
                    rRoots[0] = { r0, 1 };
                }
                return 1;
            }
            else  // delta = 0
            {
                // One real root, multiplicity 1. One real root, multiplicity
                // 2. The roots are rational numbers, so F(x) = 0 exactly for
                // each root x.
                Rational rX0 = Rational(-3, 2) * rD0 / rD1;
                Rational rX1 = Rational(-2) * rX0;
                if (rX0 < rX1)
                {
                    rRoots[0] = { rX0, 2 };
                    rRoots[1] = { rX1, 1 };
                }
                else
                {
                    rRoots[0] = { rX1, 1 };
                    rRoots[1] = { rX0, 2 };
                }
                return 2;
            }
        }
    };
}
