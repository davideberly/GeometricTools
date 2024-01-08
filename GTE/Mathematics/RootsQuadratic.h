// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.11.20

#pragma once

// Compute the real-valued roots of a quadratic polynomial with real-valued
// coefficients. The general quadratic polynomial is
//   g(x) = g0 + g1 * x + g2 * x^2
// where g2 is not zero. The monic quadratic polynomial is
//   m(x) = m0 + m1 * x + x^2
// The depressed quadratic polynomial is
//   d(x) = d0 + x^2
// The classification of roots and multiplicities is performed using rational
// arithmetic for exactness. For algorithmic details, see
// https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
//
// The code uses bisection on bounding intervals for roots. For a polynomial
// of degree n, Lagrange's bound is
//   b = max(1,|p[0]/p[n]|, |p[1]/p[n]|, ..., |p[n-1]/p[n]|)
// The real roots lie in the interval [-b,b].

#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/RootsLinear.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace gte
{
    template <typename T>
    class RootsQuadratic
    {
    public:
        // Solve for the roots using a mixture of rational arithmetic and
        // floating-point arithmetic. The roots[] array must have at least 2
        // elements. The returned size_t is the number of valid roots in the
        // roots[] array.
        using Rational = BSRational<UIntegerAP32>;

        // Solve the general quadratic g0 + g1*x + g2*x^2 = 0.
        static size_t Solve(bool useBisection,
            T const& g0, T const& g1, T const& g2,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test whether the degree is smaller than 2.
            T const zero = static_cast<T>(0);
            if (g2 == zero)
            {
                return RootsLinear<T>::Solve(g0, g1, roots);
            }

            // Test for zero-valued roots.
            if (g0 == zero)
            {
                return HasZeroValuedRoots(g1, g2, roots);
            }

            // At this time g0 and g2 are not zero. Transform the general
            // quadratic to a depressed quadratic, solve for its roots, and
            // inverse transform them to roots of the general quadratic.
            Rational rD0{}, rM1Div2{};
            ComputeClassifiers(g0, g1, g2, rD0, rM1Div2);

            std::array<PolynomialRoot<Rational>, 2> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM1Div2, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the monic quadratic m0 + m1*x + x^2 = 0.
        static size_t Solve(bool useBisection,
            T const& m0, T const& m1,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // Test for zero-valued roots.
            T const zero = static_cast<T>(0);
            if (m0 == zero)
            {
                return HasZeroValuedRoots(m1, roots);
            }

            // At this time m0 is not zero. Transform the monic quadratic to
            // a depressed quadratic, solve for its roots, and inverse
            // transform them to roots of the monic quadratic.
            Rational rD0{}, rM1Div2{};
            ComputeClassifiers(m0, m1, rD0, rM1Div2);

            std::array<PolynomialRoot<Rational>, 2> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, rD0, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x - rM1Div2, rRoots[i].m };
            }
            return numRoots;
        }

        // Solve the depressed quadratic d0 + x^2 = 0.
        static size_t Solve(bool useBisection,
            T const& d0,
            PolynomialRoot<T>* roots)
        {
            static_assert(
                std::is_floating_point<T>::value || std::is_same<T, Rational>::value,
                "Type T must be 'float', 'double', or 'Rational' ");

            // The quadratic is already depressed, so no transforming and
            // inverse transforming are necessary. A copy is required in case
            // T != Rational, in which case an implicit conversion occurs.
            std::array<PolynomialRoot<Rational>, 2> rRoots{};
            size_t numRoots = ComputeDepressedRoots(useBisection, d0, rRoots.data());
            for (size_t i = 0; i < numRoots; ++i)
            {
                roots[i] = { rRoots[i].x, rRoots[i].m };
            }
            return numRoots;
        }

        static size_t ComputeDepressedRoots(bool useBisection,
            Rational const& rD0,
            PolynomialRoot<Rational>* rRoots)
        {
            if (useBisection)
            {
                return ComputeDepressedRootsBisection(rD0, rRoots);
            }
            else
            {
                return ComputeDepressedRootsClosedForm(rD0, rRoots);
            }
        }

    private:
        // Determine whether the polynomial has zero-valued roots.
        static size_t HasZeroValuedRoots(T const& g1, T const& g2,
            PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (g1 == zero)
            {
                roots[0] = { zero, 2 };
                return 1;
            }
            else
            {
                size_t numRoots = RootsLinear<T>::Solve(g1, g2, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        static size_t HasZeroValuedRoots(T const& m1, PolynomialRoot<T>* roots)
        {
            T const zero = static_cast<T>(0);
            if (m1 == zero)
            {
                roots[0] = { zero, 2 };
                return 1;
            }
            else
            {
                size_t numRoots = RootsLinear<T>::Solve(m1, roots);
                roots[numRoots++] = { zero, 1 };
                std::sort(roots, roots + numRoots);
                return numRoots;
            }
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then g0, g1, and g2 are
        // passed by reference to this function with no conversion. If T is
        // 'float' or 'double', the compiler generates implicit conversions
        // for g0, g1, and g2 to Rational values.
        static void ComputeClassifiers(
            Rational const& rG0, Rational const& rG1, Rational const& rG2,
            Rational& rD0, Rational& rM1Div2)
        {
            Rational rM0 = rG0 / rG2;
            Rational rM1 = rG1 / rG2;
            ComputeClassifiers(rM0, rM1, rD0, rM1Div2);
        }

        // Wrapping the classifier computations in a function supports
        // conversion of type T. If T is Rational, then m0 and m1 are passed
        // by reference to this function with no conversion. If T is 'float'
        // or 'double', the compiler generates implicit conversions for
        // m0 and m1 to values.
        static void ComputeClassifiers(
            Rational const& rM0, Rational const& rM1,
            Rational& rD0, Rational& rM1Div2)
        {
            rM1Div2 = Rational(0.5) * rM1;
            rD0 = rM0 - rM1Div2 * rM1Div2;
        }

        static size_t ComputeDepressedRootsBisection(
            Rational const& rD0,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            if (signD0 > 0)
            {
                // Two non-real roots, each multiplicity 1.
                return 0;
            }

            if (signD0 == 0)
            {
                // One real root, multiplicity 2.
                rRoots[0] = { Rational(0), 2 };
                return 1;
            }

            // Two real roots, each multiplicity 1. The Cauchy bound for F(x)
            // is b = max{1,|d_0|}$. Use bisection on the interval [-b,b] to
            // estimate the roots.
            double d0 = rD0;
            double b = std::max(1.0, std::fabs(d0));
            auto F = [&d0](double x)
            {
                return gte::FMA(x, x, d0);
            };

            // Bisect on the interval [0,b]. The polynomial is an even
            // function, so we do not have to bisect on the interval [-b,0].
            double xMin = 0.0, xMax = b;
            PolynomialRootBisect<double>(F, -1, +1, xMin, xMax);
            Rational average = Rational(0.5) * (Rational(xMin) + Rational(xMax));
            rRoots[1] = { average, 1 };
            average.Negate();
            rRoots[0] = { average, 1 };
            return 2;
        }

        static size_t ComputeDepressedRootsClosedForm(
            Rational const& rD0,
            PolynomialRoot<Rational>* rRoots)
        {
            int32_t signD0 = rD0.GetSign();
            if (signD0 > 0)
            {
                // Two non-real roots, each multiplicity 1.
                return 0;
            }

            if (signD0 == 0)
            {
                // One real root, multiplicity 2.
                rRoots[0] = { Rational(0), 2 };
                return 1;
            }

            // Two real roots, each multiplicity 1. Use the closed-form
            // representation of the roots.
            Rational rSqrtNegD0 = std::sqrt(-rD0);
            rRoots[1] = { rSqrtNegD0, 1 };
            rSqrtNegD0.Negate();
            rRoots[0] = { rSqrtNegD0, 1 };
            return 2;
        }
    };
}
