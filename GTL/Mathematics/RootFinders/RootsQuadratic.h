// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Compute the real-valued roots of a quadratic polynomial with
// real-valued coefficients. For algorithmic details, see
//   https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
// The classification of roots and multiplicities is performed using
// rational arithmetic for exactness.

#include <GTL/Mathematics/Arithmetic/ArbitraryPrecision.h>
#include <GTL/Mathematics/Arithmetic/APConversion.h>
#include <cmath>
#include <map>
#include <utility>
#include <vector>

namespace gtl
{
    class RootsQuadratic
    {
    public:
        using Rational = BSRational<UIntegerAP32>;

        // The polynomial is p0 + p1 * z + p2 * z^2, where p2 is not zero.
        template <typename T>
        static void Solve(Rational const& p0, Rational const& p1,
            Rational const& p2, std::map<T, size_t>& rootMultiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p2 != rat0,
                "The coefficient p2 must not be zero.");

            Rational const rat2 = 2;
            Rational q0 = p0 / p2;
            Rational q1 = p1 / p2;
            Rational q1half = q1 / rat2;
            Rational c0 = q0 - q1half * q1half;

            std::map<Rational, size_t> rmDepressed{};
            SolveDepressed(c0, rmDepressed);

            rootMultiplicity.clear();
            for (auto const& rm : rmDepressed)
            {
                Rational root = rm.first - q1half;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root), rm.second));
            }
        }

        // The polynomial is p0 + p1 * z + p2 * z^2, where p2 is not zero.
        static void Classify(Rational const& p0, Rational const& p1,
            Rational const& p2, std::vector<size_t>& multiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p2 != rat0,
                "The coefficient p2 must not be zero.");

            Rational const rat2 = 2;
            Rational q0 = p0 / p2;
            Rational q1 = p1 / p2;
            Rational q1half = q1 / rat2;
            Rational c0 = q0 - q1half * q1half;

            std::vector<size_t> clDepressed;
            ClassifyDepressed(c0, clDepressed);
            multiplicity = std::move(clDepressed);
        }

        // The polynomial is c0 + z^2.
        template <typename T>
        static void SolveDepressed(Rational const& c0,
            std::map<T, size_t>& rootMultiplicity)
        {
            rootMultiplicity.clear();

            Rational const rat0 = 0;
            if (c0 < rat0)
            {
                // Two simple roots.
                Rational root1 = std::sqrt(-c0);
                Rational root0 = -root1;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
            }
            else if (c0 == rat0)
            {
                // One double root.
                rootMultiplicity.insert(std::make_pair(C_<T>(0), 2));
            }
            else  // c0 > 0
            {
                // A complex-conjugate pair of roots.
                // Complex z0 = -q1/2 - i*sqrt(c0);
                // Complex z0conj = -q1/2 + i*sqrt(c0);
            }
        }

        // The polynomial is c0 + z^2.
        static void ClassifyDepressed(Rational const& c0,
            std::vector<size_t>& multiplicity)
        {
            multiplicity.clear();

            Rational const rat0 = 0;
            if (c0 < rat0)
            {
                // Two simple roots.
                multiplicity.push_back(1);
                multiplicity.push_back(1);
            }
            else if (c0 == rat0)
            {
                // One double root.
                multiplicity.push_back(2);
            }
            else  // c0 > 0
            {
                // A complex-conjugate pair of roots.
            }
        }

    private:
        friend class UnitTestRootsQuadratic;
    };
}
