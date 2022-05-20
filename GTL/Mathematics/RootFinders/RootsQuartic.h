// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Compute the real-valued roots of a quartic polynomial with
// real-valued coefficients. For algorithmic details, see
//   https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
// The classification of roots and multiplicities is performed using
// rational arithmetic for exactness.

#include <GTL/Mathematics/RootFinders/RootsCubic.h>

namespace gtl
{
    class RootsQuartic
    {
    public:
        using Rational = BSRational<UIntegerAP32>;

        // The polynomial is p0 + p1 * z + p2 * z^2 + p3 * z^3 + p4 * z^4,
        // where p4 is not zero. The rootMultiplicity map must be empty when
        // calling this function externally. The requirement is waived
        // internally.
        template <typename T>
        static void Solve(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, Rational const& p4,
            std::map<T, size_t>& rootMultiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p4 != rat0,
                "The coefficient p4 must not be zero.");

            Rational const rat2 = 2, rat3 = 3, rat4 = 4, rat6 = 6;
            Rational q0 = p0 / p4;
            Rational q1 = p1 / p4;
            Rational q2 = p2 / p4;
            Rational q3 = p3 / p4;
            Rational q3fourth = q3 / rat4;
            Rational q3fourthSqr = q3fourth * q3fourth;
            Rational c0 = q0 - q3fourth * (q1 - q3fourth * (q2 - q3fourthSqr * rat3));
            Rational c1 = q1 - rat2 * q3fourth * (q2 - rat4 * q3fourthSqr);
            Rational c2 = q2 - rat6 * q3fourthSqr;

            std::map<Rational, size_t> rmDepressed;
            SolveDepressed(c0, c1, c2, rmDepressed);

            rootMultiplicity.clear();
            for (auto const& rm : rmDepressed)
            {
                Rational root = rm.first - q3fourth;
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root), rm.second));
            }
        }

        // The polynomial is p0 + p1 * z + p2 * z^2 + p3 * z^3 + p4 * z^4,
        // where p4 is not zero. The multiplicity vector must be empty when
        // calling this function externally. The requirement is waived
        // internally.
        static void Classify(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, Rational const& p4,
            std::vector<size_t>& multiplicity)
        {
            Rational const rat0 = 0;
            GTL_ARGUMENT_ASSERT(
                p4 != rat0,
                "The coefficient p4 must not be zero.");

            Rational const rat2 = 2, rat3 = 3, rat4 = 4, rat6 = 6;
            Rational q0 = p0 / p4;
            Rational q1 = p1 / p4;
            Rational q2 = p2 / p4;
            Rational q3 = p3 / p4;
            Rational q3fourth = q3 / rat4;
            Rational q3fourthSqr = q3fourth * q3fourth;
            Rational c0 = q0 - q3fourth * (q1 - q3fourth * (q2 - q3fourthSqr * rat3));
            Rational c1 = q1 - rat2 * q3fourth * (q2 - rat4 * q3fourthSqr);
            Rational c2 = q2 - rat6 * q3fourthSqr;

            ClassifyDepressed(c0, c1, c2, multiplicity);
        }

        // The polynomial is c0 + c1 * z + c2 * z^2 + z^4. The
        // rootMultiplicity map must be empty when calling this function
        // externally. The requirement is waived internally.
        template <typename T>
        static void SolveDepressed(Rational const& c0, Rational const& c1,
            Rational const& c2, std::map<T, size_t>& rootMultiplicity)
        {
            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed cubic.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                RootsCubic::SolveDepressed(c1, c2, rootMultiplicity);
                auto iter = rootMultiplicity.find(C_<T>(0));
                if (iter != rootMultiplicity.end())
                {
                    // The cubic has a root of zero, so the multiplicity must
                    // be increased.
                    ++iter->second;
                }
                else
                {
                    // The cubic does not have a root of zero. Insert the one
                    // for the quartic.
                    rootMultiplicity.insert(std::make_pair(C_<T>(0), 1));
                }
                return;
            }

            // Handle the special case of c1 = 0, in which case the quartic is
            // a biquadratic
            //   x^4 + c2*x^2 + c0 = (x^2 + c2/2)^2 + (c0 - c2^2/4)
            if (c1 == rat0)
            {
                SolveBiquadratic(c0, c2, rootMultiplicity);
                return;
            }

            // At this time, c0 != 0 and c1 != 0, which is a requirement for
            // the general solver that must use a root of a special cubic
            // polynomial.
            Rational const rat2 = 2, rat4 = 4, rat8 = 8, rat12 = 12, rat16 = 16;
            Rational const rat27 = 27, rat36 = 36;
            Rational c0sqr = c0 * c0, c1sqr = c1 * c1, c2sqr = c2 * c2;
            Rational delta = c1sqr * (-rat27 * c1sqr + rat4 * c2 *
                (rat36 * c0 - c2sqr)) + rat16 * c0 * (c2sqr * (c2sqr - rat8 * c0) +
                    rat16 * c0sqr);
            Rational a0 = rat12 * c0 + c2sqr;
            Rational a1 = rat4 * c0 - c2sqr;

            if (delta > rat0)
            {
                if (c2 < rat0 && a1 < rat0)
                {
                    // Four simple real roots.
                    std::map<Rational, size_t> rmCubic;
                    RootsCubic::Solve(c1sqr - rat4 * c0 * c2, rat8 * c0, rat4 * c2, -rat8, rmCubic);
                    Rational t = rmCubic.rbegin()->first;
                    Rational alphaSqr = rat2 * t - c2;
                    Rational alpha = std::sqrt(alphaSqr);
                    Rational sgnC1;
                    if (c1 > rat0)
                    {
                        sgnC1 = 1;
                    }
                    else
                    {
                        sgnC1 = -1;
                    }
                    Rational arg = t * t - c0;
                    Rational beta = sgnC1 * std::sqrt(std::max(arg, rat0));
                    Rational D0 = alphaSqr - rat4 * (t + beta);
                    Rational sqrtD0 = std::sqrt(std::max(D0, rat0));
                    Rational D1 = alphaSqr - rat4 * (t - beta);
                    Rational sqrtD1 = std::sqrt(std::max(D1, rat0));
                    Rational root0 = (alpha - sqrtD0) / rat2;
                    Rational root1 = (alpha + sqrtD0) / rat2;
                    Rational root2 = (-alpha - sqrtD1) / rat2;
                    Rational root3 = (-alpha + sqrtD1) / rat2;
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root2), 1));
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root3), 1));
                }
                else // c2 >= 0 or a1 >= 0
                {
                    // Two complex-conjugate pairs.  The values alpha, D0
                    // and D1 are those of the if-block.
                    // Complex z0 = (alpha - i*sqrt(-D0))/2;
                    // Complex z0conj = (alpha + i*sqrt(-D0))/2;
                    // Complex z1 = (-alpha - i*sqrt(-D1))/2;
                    // Complex z1conj = (-alpha + i*sqrt(-D1))/2;
                }
            }
            else if (delta < rat0)
            {
                // Two simple real roots, one complex-conjugate pair.
                std::map<Rational, size_t> rmCubic;
                RootsCubic::Solve(c1sqr - rat4 * c0 * c2, rat8 * c0, rat4 * c2, -rat8, rmCubic);
                Rational t = rmCubic.rbegin()->first;
                Rational alphaSqr = rat2 * t - c2;
                Rational alpha = std::sqrt(std::max(alphaSqr, rat0));
                Rational sgnC1;
                if (c1 > rat0)
                {
                    sgnC1 = 1;  // leads to BLOCK(18)
                }
                else
                {
                    sgnC1 = -1;  // leads to BLOCK(19)
                }
                Rational arg = t * t - c0;
                Rational beta = sgnC1 * std::sqrt(std::max(arg, rat0));
                Rational root0, root1;
                if (sgnC1 > rat0)
                {
                    Rational D1 = alphaSqr - rat4 * (t - beta);
                    Rational sqrtD1 = std::sqrt(std::max(D1, rat0));
                    root0 = (-alpha - sqrtD1) / rat2;
                    root1 = (-alpha + sqrtD1) / rat2;

                    // One complex conjugate pair.
                    // Complex z0 = (alpha - i*sqrt(-D0))/2;
                    // Complex z0conj = (alpha + i*sqrt(-D0))/2;
                }
                else
                {
                    Rational D0 = alphaSqr - rat4 * (t + beta);
                    Rational sqrtD0 = std::sqrt(std::max(D0, rat0));
                    root0 = (alpha - sqrtD0) / rat2;
                    root1 = (alpha + sqrtD0) / rat2;

                    // One complex conjugate pair.
                    // Complex z0 = (-alpha - i*sqrt(-D1))/2;
                    // Complex z0conj = (-alpha + i*sqrt(-D1))/2;
                }
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
            }
            else  // delta = 0
            {
                if (a1 > rat0 || (c2 > rat0 && (a1 != rat0 || c1 != rat0)))
                {
                    // One double real root, one complex-conjugate pair.
                    Rational const rat9 = 9;
                    Rational root0 = -c1 * a0 / (rat9 * c1sqr - rat2 * c2 * a1);
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 2));

                    // One complex conjugate pair.
                    // Complex z0 = -root0 - i*sqrt(c2 + root0^2);
                    // Complex z0conj = -root0 + i*sqrt(c2 + root0^2);
                }
                else
                {
                    Rational const rat3 = 3;
                    if (a0 != rat0)
                    {
                        // One double real root, two simple real roots.
                        Rational const rat9 = 9;
                        Rational root0 = -c1 * a0 / (rat9 * c1sqr - rat2 * c2 * a1);
                        Rational alpha = rat2 * root0;
                        Rational beta = c2 + rat3 * root0 * root0;
                        Rational discr = alpha * alpha - rat4 * beta;
                        Rational temp1 = std::sqrt(discr);
                        Rational root1 = (-alpha - temp1) / rat2;
                        Rational root2 = (-alpha + temp1) / rat2;
                        rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 2));
                        rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
                        rootMultiplicity.insert(std::make_pair(static_cast<T>(root2), 1));
                    }
                    else
                    {
                        // One triple real root, one simple real root.
                        Rational root0 = -rat3 * c1 / (rat4 * c2);
                        Rational root1 = -rat3 * root0;
                        rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 3));
                        rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
                    }
                }
            }
        }

        // The polynomial is c0 + c1 * z + c2 * z^2 + z^4. The multiplicity
        // vector must be empty when calling this function/ externally. The
        // requirement is waived internally.
        static void ClassifyDepressed(Rational const& c0, Rational const& c1,
            Rational const& c2, std::vector<size_t>& multiplicity)
        {
            multiplicity.clear();

            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed cubic.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                if (c1 == rat0)
                {
                    if (c2 == rat0)
                    {
                        multiplicity.push_back(4);  // quadruple root of zero
                    }
                    else
                    {
                        RootsQuadratic::ClassifyDepressed(c2, multiplicity);
                        multiplicity.push_back(2);  // double root of zero
                    }
                }
                else
                {
                    RootsCubic::ClassifyDepressed(c1, c2, multiplicity);
                    multiplicity.push_back(1);  // simple root of zero
                }
                return;
            }

            // Handle the special case of c1 = 0, in which case the quartic is
            // a biquadratic
            //   x^4 + c1*x^2 + c0 = (x^2 + c2/2)^2 + (c0 - c2^2/4)
            if (c1 == rat0)
            {
                ClassifyBiquadratic(c0, c2, multiplicity);
                return;
            }

            // At this time, c0 != 0 and c1 != 0, which is a requirement for
            // the general solver that must use a root of a special cubic
            // polynomial.
            Rational const rat4 = 4, rat8 = 8, rat12 = 12, rat16 = 16;
            Rational const rat27 = 27, rat36 = 36;
            Rational c0sqr = c0 * c0, c1sqr = c1 * c1, c2sqr = c2 * c2;
            Rational delta = c1sqr * (-rat27 * c1sqr + rat4 * c2 *
                (rat36 * c0 - c2sqr)) + rat16 * c0 * (c2sqr * (c2sqr - rat8 * c0) +
                    rat16 * c0sqr);
            Rational a0 = rat12 * c0 + c2sqr;
            Rational a1 = rat4 * c0 - c2sqr;

            if (delta > rat0)
            {
                if (c2 < rat0 && a1 < rat0)
                {
                    // Four simple real roots.
                    multiplicity.push_back(1);
                    multiplicity.push_back(1);
                    multiplicity.push_back(1);
                    multiplicity.push_back(1);
                }
                else // c2 >= 0 or a1 >= 0
                {
                    // Two complex-conjugate pairs.
                }
            }
            else if (delta < rat0)
            {
                // Two simple real roots, one complex-conjugate pair.
                multiplicity.push_back(1);
                multiplicity.push_back(1);
            }
            else  // delta = 0
            {
                if (a1 > rat0 || (c2 > rat0 && (a1 != rat0 || c1 != rat0)))
                {
                    // One double real root, one complex-conjugate pair.
                    multiplicity.push_back(2);
                }
                else
                {
                    if (a0 != rat0)
                    {
                        // One double real root, two simple real roots.
                        multiplicity.push_back(2);
                        multiplicity.push_back(1);
                        multiplicity.push_back(1);
                    }
                    else
                    {
                        // One triple real root, one simple real root.
                        multiplicity.push_back(3);
                        multiplicity.push_back(1);
                    }
                }
            }
        }

        // The polynomial is c0 + c2 * z^2 + z^4. The multiplicity vector must
        // be empty when calling this function externally. The requirement is
        // waived internally.
        template <typename T>
        static void SolveBiquadratic(Rational const& c0, Rational const& c2,
            std::map<T, size_t>& rootMultiplicity)
        {
            rootMultiplicity.clear();

            // Solve 0 = z^4 + c2*z^2 + c0 = (z^2 + c2/2)^2 + b0, where
            // b0 = c0 - c2^2/4.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                // The polynomial is z^4 + c2*z^2 = z^2 * (z^2 + c2).
                RootsQuadratic::SolveDepressed(c2, rootMultiplicity);
                auto iter = rootMultiplicity.find(C_<T>(0));
                if (iter != rootMultiplicity.end())
                {
                    // The quadratic has a root of zero, so the multiplicity
                    // must be increased.
                    iter->second += 2;
                }
                else
                {
                    // The quadratic does not have a root of zero. Insert the
                    // one for the quartic.
                    rootMultiplicity.insert(std::make_pair(C_<T>(0), 2));
                }
                return;
            }

            // At this time, c0 != 0, c1 = 0, b0 = c0 - c2^2/4 and
            // delta = 256*c0^3 - 128*c2^2*c0^2 + 16*c2^4*c0 = 256*c0*b0^2.
            // Therefore, sign(delta) = sign(c0) * sign(b0^2). If b0 = 0,
            // then delta = 0. If b0 != 0, then sign(delta) = sign(c0).
            Rational const rat2 = 2;
            Rational c2Half = c2 / rat2;
            Rational b0 = c0 - c2Half * c2Half;
            if (b0 != rat0)
            {
                if (c0 > rat0)  // delta > 0
                {
                    if (c2 < rat0)
                    {
                        if (b0 < rat0)
                        {
                            // Four simple roots.
                            Rational temp0 = std::sqrt(-b0);
                            Rational temp1 = -c2Half - temp0;
                            Rational temp2 = -c2Half + temp0;
                            Rational root1 = std::sqrt(temp1);
                            Rational root0 = -root1;
                            Rational root2 = std::sqrt(temp2);
                            Rational root3 = -root2;
                            rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                            rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));
                            rootMultiplicity.insert(std::make_pair(static_cast<T>(root2), 1));
                            rootMultiplicity.insert(std::make_pair(static_cast<T>(root3), 1));
                        }
                        else  // b0 > 0
                        {
                            // Two simple complex conjugate pairs.
                            // double thetaDiv2 = atan2(sqrt(b0), -c2/2) / 2.0;
                            // double cs = cos(thetaDiv2), sn = sin(thetaDiv2);
                            // double length = pow(c0, 0.25);
                            // Complex z0 = length*(cs + i*sn);
                            // Complex z0conj = length*(cs - i*sn);
                            // Complex z1 = length*(-cs + i*sn);
                            // Complex z1conj = length*(-cs - i*sn);
                        }
                    }
                    else  // c2 >= 0
                    {
                        // Two simple complex conjugate pairs.
                        // Complex z0 = -i*sqrt(c2/2 - sqrt(-b0));
                        // Complex z0conj = +i*sqrt(c2/2 - sqrt(-b0));
                        // Complex z1 = -i*sqrt(c2/2 + sqrt(-b0));
                        // Complex z1conj = +i*sqrt(c2/2 + sqrt(-b0));
                    }
                }
                else // delta < 0
                {
                    // Two simple real roots.
                    Rational temp0 = std::sqrt(-b0);
                    Rational temp1 = -c2Half + temp0;
                    Rational root1 = std::sqrt(temp1);
                    Rational root0 = -root1;
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 1));
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 1));

                    // One complex conjugate pair.
                    // Complex z0 = -i*sqrt(c2/2 + sqrt(-b0));
                    // Complex z0conj = +i*sqrt(c2/2 + sqrt(-b0));
                }
            }
            else  // delta = 0
            {
                if (c2 < rat0)
                {
                    // Two double real roots.
                    Rational root1 = std::sqrt(-c2Half);
                    Rational root0 = -root1;
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root0), 2));
                    rootMultiplicity.insert(std::make_pair(static_cast<T>(root1), 2));
                }
                else  // c2 > 0
                {
                    // Two double complex conjugate pairs.
                    // Complex z0 = -i*sqrt(c2/2);  // multiplicity 2
                    // Complex z0conj = +i*sqrt(c2/2);  // multiplicity 2
                }
            }
        }

        // The polynomial is c0 + c2 * z^2 + z^4. The rootMultiplicity map
        // must be empty when calling this function externally. The
        // requirement is waived internally.
        template <typename Rational>
        static void ClassifyBiquadratic(Rational const& c0,
            Rational const& c2, std::vector<size_t>& multiplicity)
        {
            multiplicity.clear();

            // Solve 0 = x^4 + c2*x^2 + c0 = (x^2 + c2/2)^2 + b0, where
            // b0 = c0 - c2^2/4.
            Rational const rat0 = 0;
            if (c0 == rat0)
            {
                // The polynomial is z^4 + c2*z^2 = z^2 * (z^2 + c2).
                if (c2 == rat0)
                {
                    // One root of multiplicity 4.
                    multiplicity.push_back(4);
                }
                else
                {
                    // One root of multiplicity 2, two simple roots.
                    multiplicity.push_back(2);
                    multiplicity.push_back(1);
                    multiplicity.push_back(1);
                }
                return;
            }

            // At this time, c0 != 0, c1 = 0, b0 = c0 - c2^2/4 and
            // delta = 256*c0^3 - 128*c2^2*c0^2 + 16*c2^4*c0 = 256*c0*b0^2.
            // Therefore, sign(delta) = sign(c0) * sign(b0^2). If b0 = 0,
            // then delta = 0. If b0 != 0, then sign(delta) = sign(c0).
            Rational const rat2 = 2;
            Rational c2Half = c2 / rat2;
            Rational b0 = c0 - c2Half * c2Half;
            if (b0 != rat0)
            {
                if (c0 > rat0) // delta > 0
                {
                    if (c2 < rat0)
                    {
                        if (b0 < rat0)
                        {
                            // Four simple roots.
                            multiplicity.push_back(1);
                            multiplicity.push_back(1);
                            multiplicity.push_back(1);
                            multiplicity.push_back(1);
                        }
                        else  // b0 > 0
                        {
                            // Two simple complex conjugate pairs.
                        }
                    }
                    else  // c2 >= 0
                    {
                        // Two simple complex conjugate pairs.
                    }
                }
                else  // delta < 0
                {
                    // Two simple real roots, one complex conjugate pair.
                    multiplicity.push_back(1);
                    multiplicity.push_back(1);
                }
            }
            else  // delta = 0
            {
                if (c2 < rat0)
                {
                    // Two double real roots.
                    multiplicity.push_back(2);
                    multiplicity.push_back(2);
                }
                else  // c2 > 0
                {
                    // Two double complex conjugate pairs.
                }
            }
        }

    private:
        friend class UnitTestRootsQuartic;
    };
}
