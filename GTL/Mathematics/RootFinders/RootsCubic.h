// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Compute the real-valued roots of a cubic polynomial with
// real-valued coefficients. For algorithmic details, see
//   https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
// The classification of roots and multiplicities is performed using
// rational arithmetic for exactness.

#include <GTL/Mathematics/RootFinders/RootsQuadratic.h>
#include <cmath>
#include <map>
#include <utility>
#include <vector>

namespace gtl
{
    class RootsCubic
    {
    public:
        using Rational = BSRational<UIntegerAP32>;

        // The polynomial is p0 + p1 * z + p2 * z^2 + p3 * z^3, where p3 is
        // not zero.
        template <typename T>
        static void Solve(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3,
            std::map<T, size_t>& rootMultiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p3 != rat0,
                "The coefficient p3 must not be zero.");

            Rational const rat2 = 2, rat3 = 3;
            Rational q0 = p0 / p3;
            Rational q1 = p1 / p3;
            Rational q2 = p2 / p3;
            Rational q2third = q2 / rat3;
            Rational c0 = q0 - q2third * (q1 - rat2 * q2third * q2third);
            Rational c1 = q1 - q2 * q2third;

            std::map<Rational, size_t> rmDepressed{};
            SolveDepressed(c0, c1, rmDepressed);

            rootMultiplicity.clear();
            for (auto const& rm : rmDepressed)
            {
                Rational root = rm.first - q2third;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root), rm.second));
            }
        }

        // The polynomial is p0 + p1 * z + p2 * z^2 + p3 * z^3, where p3 is
        // not zero.
        static void Classify(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3,
            std::vector<size_t>& multiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p3 != rat0,
                "The coefficient p3 must not be zero.");

            Rational const rat2 = 2, rat3 = 3;
            Rational q0 = p0 / p3;
            Rational q1 = p1 / p3;
            Rational q2 = p2 / p3;
            Rational q2third = q2 / rat3;
            Rational c0 = q0 - q2third * (q1 - rat2 * q2third * q2third);
            Rational c1 = q1 - q2 * q2third;

            ClassifyDepressed(c0, c1, multiplicity);
        }

        // The polynomial is c0 + c1 * z + z^3.
        template <typename T>
        static void SolveDepressed(Rational const& c0, Rational const& c1,
            std::map<T, size_t>& rootMultiplicity)
        {
            rootMultiplicity.clear();

            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed quadratic.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                RootsQuadratic::SolveDepressed(c1, rootMultiplicity);
                auto iter = rootMultiplicity.find(C_<T>(0));
                if (iter != rootMultiplicity.end())
                {
                    // The quadratic has a root of zero, so the multiplicity
                    // must be increased.
                    ++iter->second;
                }
                else
                {
                    // The quadratic does not have a root of zero. Insert the
                    // one for the cubic.
                    rootMultiplicity.insert(std::make_pair(C_<T>(0), 1));
                }
                return;
            }

            // Handle the special case of c0 != 0 and c1 = 0.
            Rational const rat1 = 1, rat3 = 3;
            Rational const ratOneThird = rat1 / rat3;
            if (c1 == rat0)
            {
                // One simple real root.
                Rational root0;
                if (c0 > rat0)
                {
                    root0 = -std::pow(c0, ratOneThird);
                }
                else
                {
                    root0 = std::pow(-c0, ratOneThird);
                }
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));

                // One complex conjugate pair.
                // Complex z0 = root0*(-1 - i*sqrt(3))/2;
                // Complex z0conj = root0*(-1 + i*sqrt(3))/2;
                return;
            }

            // At this time, c0 != 0 and c1 != 0.
            Rational const rat2 = 2, rat4 = 4, rat27 = 27, rat108 = 108;
            Rational delta = -(rat4 * c1 * c1 * c1 + rat27 * c0 * c0);
            if (delta > rat0)
            {
                // Three simple roots.
                Rational const rat6 = 6, ratOneSixth = rat1 / rat6;
                Rational deltaDiv108 = delta / rat108;
                Rational betaRe = -c0 / rat2;
                Rational betaIm = std::sqrt(deltaDiv108);
                Rational theta = std::atan2(betaIm, betaRe);
                Rational thetaDiv3 = theta / rat3;
                Rational cs = std::cos(thetaDiv3);
                Rational sn = std::sin(thetaDiv3);
                Rational rhoSqr = betaRe * betaRe + betaIm * betaIm;
                Rational rhoPowThird = std::pow(rhoSqr, ratOneSixth);
                Rational temp0 = rhoPowThird * cs;
                Rational temp1 = rhoPowThird * sn * std::sqrt(rat3);
                Rational root0 = rat2 * temp0;
                Rational root1 = -temp0 - temp1;
                Rational root2 = -temp0 + temp1;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root2), 1));
            }
            else if (delta < rat0)
            {
                // One simple root.
                Rational deltaDiv108 = delta / rat108;
                Rational temp0 = -c0 / rat2;
                Rational temp1 = std::sqrt(-deltaDiv108);
                Rational temp2 = temp0 - temp1;
                Rational temp3 = temp0 + temp1;
                if (temp2 >= rat0)
                {
                    temp2 = std::pow(temp2, ratOneThird);
                }
                else
                {
                    temp2 = -std::pow(-temp2, ratOneThird);
                }
                if (temp3 >= rat0)
                {
                    temp3 = std::pow(temp3, ratOneThird);
                }
                else
                {
                    temp3 = -std::pow(-temp3, ratOneThird);
                }
                Rational root0 = temp2 + temp3;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));

                // One complex conjugate pair.
                // Complex z0 = (-root0 - i*sqrt(3*root0*root0+4*c1))/2;
                // Complex z0conj = (-root0 + i*sqrt(3*root0*root0+4*c1))/2;
            }
            else  // delta = 0
            {
                // One simple root and one double root.
                Rational root0 = -rat3 * c0 / (rat2 * c1);
                Rational root1 = -rat2 * root0;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 2));
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
            }
        }

        // The polynomial is c0 + c1 * z + z^3.
        static void ClassifyDepressed(Rational const& c0,
            Rational const& c1, std::vector<size_t>& multiplicity)
        {
            multiplicity.clear();

            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed quadratic.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                if (c1 == rat0)
                {
                    multiplicity.push_back(3);  // triple root of zero
                }
                else
                {
                    RootsQuadratic::ClassifyDepressed(c1, multiplicity);
                    multiplicity.push_back(1);  // simple root of zero
                }
                return;
            }

            Rational const rat4 = 4, rat27 = 27;
            Rational delta = -(rat4 * c1 * c1 * c1 + rat27 * c0 * c0);
            if (delta > rat0)
            {
                // Three simple real roots.
                multiplicity.push_back(1);
                multiplicity.push_back(1);
                multiplicity.push_back(1);
            }
            else if (delta < rat0)
            {
                // One simple real root.
                multiplicity.push_back(1);
            }
            else  // delta = 0
            {
                // One simple real root and one double real root.
                multiplicity.push_back(1);
                multiplicity.push_back(2);
            }
        }

    private:
        friend class UnitTestRootsCubic;
    };
}
