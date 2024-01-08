// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.11.20

#pragma once

// The Find functions return the number of roots, if any, and this number
// of elements of the outputs are valid.  If the polynomial is identically
// zero, Find returns 1.
//
// Some root-bounding algorithms for real-valued roots are mentioned next for
// the polynomial p(t) = c[0] + c[1]*t + ... + c[d-1]*t^{d-1} + c[d]*t^d.
//
// 1. The roots must be contained by the interval [-M,M] where
//   M = 1 + max{|c[0]|, ..., |c[d-1]|}/|c[d]| >= 1
// is called the Cauchy bound.
//
// 2. You may search for roots in the interval [-1,1].  Define
//   q(t) = t^d*p(1/t) = c[0]*t^d + c[1]*t^{d-1} + ... + c[d-1]*t + c[d]
// The roots of p(t) not in [-1,1] are the roots of q(t) in [-1,1].
//
// 3. Between two consecutive roots of the derivative p'(t), say, r0 < r1,
// the function p(t) is strictly monotonic on the open interval (r0,r1).
// If additionally, p(r0) * p(r1) <= 0, then p(x) has a unique root on
// the closed interval [r0,r1].  Thus, one can compute the derivatives
// through order d for p(t), find roots for the derivative of order k+1,
// then use these to bound roots for the derivative of order k.
//
// 4. Sturm sequences of polynomials may be used to determine bounds on the
// roots.  This is a more sophisticated approach to root bounding than item 3.
// Moreover, a Sturm sequence allows you to compute the number of real-valued
// roots on a specified interval.
//
// 5. For the low-degree Solve* functions, see
// https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf

// FOR INTERNAL USE ONLY (unit testing).  Do not define the symbol
// GTE_ROOTS_LOW_DEGREE_UNIT_TEST in your own code.
#if defined(GTE_ROOTS_LOW_DEGREE_UNIT_TEST)
extern void RootsLowDegreeBlock(int32_t);
#define GTE_ROOTS_LOW_DEGREE_BLOCK(block) RootsLowDegreeBlock(block)
#else
#define GTE_ROOTS_LOW_DEGREE_BLOCK(block)
#endif

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

// NOTE: RootsPolynomial is deprecated. For low-degree root finders, use
// RootsLinear, RootsQuadratic, RootsCubic, and RootsQuartic. For general
// degree, use RootsGeneralPolynomial.

namespace gte
{
    template <typename Real>
    class RootsPolynomial // [deprecated("Use RootsGeneralPolynomial instead.")]
    {
    public:
        // Low-degree root finders.  These use exact rational arithmetic for
        // theoretically correct root classification.  The roots themselves
        // are computed with mixed types (rational and floating-point
        // arithmetic).  The Rational type must support rational arithmetic
        // (+, -, *, /); for example, BSRational<UIntegerAP32> suffices.  The
        // Rational class must have single-input constructors where the input
        // is type Real.  This ensures you can call the Solve* functions with
        // floating-point inputs; they will be converted to Rational
        // implicitly.  The highest-order coefficients must be nonzero
        // (p2 != 0 for quadratic, p3 != 0 for cubic, and p4 != 0 for
        // quartic).

        template <typename Rational>
        static void SolveQuadratic(Rational const& p0, Rational const& p1,
            Rational const& p2, std::map<Real, int32_t>& rmMap)
        {
            Rational const rat2 = 2;
            Rational q0 = p0 / p2;
            Rational q1 = p1 / p2;
            Rational q1half = q1 / rat2;
            Rational c0 = q0 - q1half * q1half;

            std::map<Rational, int32_t> rmLocalMap;
            SolveDepressedQuadratic(c0, rmLocalMap);

            rmMap.clear();
            for (auto& rm : rmLocalMap)
            {
                Rational root = rm.first - q1half;
                rmMap.insert(std::make_pair((Real)root, rm.second));
            }
        }

        template <typename Rational>
        static void SolveCubic(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, std::map<Real, int32_t>& rmMap)
        {
            Rational const rat2 = 2, rat3 = 3;
            Rational q0 = p0 / p3;
            Rational q1 = p1 / p3;
            Rational q2 = p2 / p3;
            Rational q2third = q2 / rat3;
            Rational c0 = q0 - q2third * (q1 - rat2 * q2third * q2third);
            Rational c1 = q1 - q2 * q2third;

            std::map<Rational, int32_t> rmLocalMap;
            SolveDepressedCubic(c0, c1, rmLocalMap);

            rmMap.clear();
            for (auto& rm : rmLocalMap)
            {
                Rational root = rm.first - q2third;
                rmMap.insert(std::make_pair((Real)root, rm.second));
            }
        }

        template <typename Rational>
        static void SolveQuartic(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, Rational const& p4,
            std::map<Real, int32_t>& rmMap)
        {
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

            std::map<Rational, int32_t> rmLocalMap;
            SolveDepressedQuartic(c0, c1, c2, rmLocalMap);

            rmMap.clear();
            for (auto& rm : rmLocalMap)
            {
                Rational root = rm.first - q3fourth;
                rmMap.insert(std::make_pair((Real)root, rm.second));
            }
        }

        // Return only the number of real-valued roots and their
        // multiplicities.  info.size() is the number of real-valued roots
        // and info[i] is the multiplicity of root corresponding to index i.
        template <typename Rational>
        static void GetRootInfoQuadratic(Rational const& p0, Rational const& p1,
            Rational const& p2, std::vector<int32_t>& info)
        {
            Rational const rat2 = 2;
            Rational q0 = p0 / p2;
            Rational q1 = p1 / p2;
            Rational q1half = q1 / rat2;
            Rational c0 = q0 - q1half * q1half;

            info.clear();
            info.reserve(2);
            GetRootInfoDepressedQuadratic(c0, info);
        }

        template <typename Rational>
        static void GetRootInfoCubic(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, std::vector<int32_t>& info)
        {
            Rational const rat2 = 2, rat3 = 3;
            Rational q0 = p0 / p3;
            Rational q1 = p1 / p3;
            Rational q2 = p2 / p3;
            Rational q2third = q2 / rat3;
            Rational c0 = q0 - q2third * (q1 - rat2 * q2third * q2third);
            Rational c1 = q1 - q2 * q2third;

            info.clear();
            info.reserve(3);
            GetRootInfoDepressedCubic(c0, c1, info);
        }

        template <typename Rational>
        static void GetRootInfoQuartic(Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& p3, Rational const& p4,
            std::vector<int32_t>& info)
        {
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

            info.clear();
            info.reserve(4);
            GetRootInfoDepressedQuartic(c0, c1, c2, info);
        }

        // General equations: sum_{i=0}^{d} c(i)*t^i = 0.  The input array 'c'
        // must have at least d+1 elements and the output array 'root' must
        // have at least d elements.

        // Find the roots on (-infinity,+infinity).
        static int32_t Find(int32_t degree, Real const* c, uint32_t maxIterations, Real* roots)
        {
            if (degree >= 0 && c)
            {
                Real const zero = (Real)0;
                while (degree >= 0 && c[degree] == zero)
                {
                    --degree;
                }

                if (degree > 0)
                {
                    // Compute the Cauchy bound.
                    Real const one = (Real)1;
                    Real invLeading = one / c[degree];
                    Real maxValue = zero;
                    for (int32_t i = 0; i < degree; ++i)
                    {
                        Real value = std::fabs(c[i] * invLeading);
                        if (value > maxValue)
                        {
                            maxValue = value;
                        }
                    }
                    Real bound = one + maxValue;

                    return FindRecursive(degree, c, -bound, bound, maxIterations,
                        roots);
                }
                else if (degree == 0)
                {
                    // The polynomial is a nonzero constant.
                    return 0;
                }
                else
                {
                    // The polynomial is identically zero.
                    roots[0] = zero;
                    return 1;
                }
            }
            else
            {
                // Invalid degree or c.
                return 0;
            }
        }

        // If you know that p(tmin) * p(tmax) <= 0, then there must be at
        // least one root in [tmin, tmax].  Compute it using bisection.
        static bool Find(int32_t degree, Real const* c, Real tmin, Real tmax,
            uint32_t maxIterations, Real& root)
        {
            Real const zero = (Real)0;
            Real pmin = Evaluate(degree, c, tmin);
            if (pmin == zero)
            {
                root = tmin;
                return true;
            }
            Real pmax = Evaluate(degree, c, tmax);
            if (pmax == zero)
            {
                root = tmax;
                return true;
            }

            if (pmin * pmax > zero)
            {
                // It is not known whether the interval bounds a root.
                return false;
            }

            if (tmin >= tmax)
            {
                // Invalid ordering of interval endpoitns. 
                return false;
            }

            for (uint32_t i = 1; i <= maxIterations; ++i)
            {
                root = ((Real)0.5) * (tmin + tmax);

                // This test is designed for 'float' or 'double' when tmin
                // and tmax are consecutive floating-point numbers.
                if (root == tmin || root == tmax)
                {
                    break;
                }

                Real p = Evaluate(degree, c, root);
                Real product = p * pmin;
                if (product < zero)
                {
                    tmax = root;
                    pmax = p;
                }
                else if (product > zero)
                {
                    tmin = root;
                    pmin = p;
                }
                else
                {
                    break;
                }
            }

            return true;
        }

    private:
        // Support for the Solve* functions.
        template <typename Rational>
        static void SolveDepressedQuadratic(Rational const& c0,
            std::map<Rational, int32_t>& rmMap)
        {
            Rational const zero = 0;
            if (c0 < zero)
            {
                // Two simple roots.
                Rational root1 = (Rational)std::sqrt((double)-c0);
                Rational root0 = -root1;
                rmMap.insert(std::make_pair(root0, 1));
                rmMap.insert(std::make_pair(root1, 1));
                GTE_ROOTS_LOW_DEGREE_BLOCK(0);
            }
            else if (c0 == zero)
            {
                // One double root.
                rmMap.insert(std::make_pair(zero, 2));
                GTE_ROOTS_LOW_DEGREE_BLOCK(1);
            }
            else  // c0 > 0
            {
                // A complex-conjugate pair of roots.
                // Complex z0 = -q1/2 - i*sqrt(c0);
                // Complex z0conj = -q1/2 + i*sqrt(c0);
                GTE_ROOTS_LOW_DEGREE_BLOCK(2);
            }
        }

        template <typename Rational>
        static void SolveDepressedCubic(Rational const& c0, Rational const& c1,
            std::map<Rational, int32_t>& rmMap)
        {
            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed quadratic.
            Rational const zero = 0;
            if (c0 == zero)
            {
                SolveDepressedQuadratic(c1, rmMap);
                auto iter = rmMap.find(zero);
                if (iter != rmMap.end())
                {
                    // The quadratic has a root of zero, so the multiplicity
                    // must be increased.
                    ++iter->second;
                    GTE_ROOTS_LOW_DEGREE_BLOCK(3);
                }
                else
                {
                    // The quadratic does not have a root of zero.  Insert the
                    // one for the cubic.
                    rmMap.insert(std::make_pair(zero, 1));
                    GTE_ROOTS_LOW_DEGREE_BLOCK(4);
                }
                return;
            }

            // Handle the special case of c0 != 0 and c1 = 0.
            double const oneThird = 1.0 / 3.0;
            if (c1 == zero)
            {
                // One simple real root.
                Rational root0;
                if (c0 > zero)
                {
                    root0 = (Rational)-std::pow((double)c0, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(5);
                }
                else
                {
                    root0 = (Rational)std::pow(-(double)c0, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(6);
                }
                rmMap.insert(std::make_pair(root0, 1));

                // One complex conjugate pair.
                // Complex z0 = root0*(-1 - i*sqrt(3))/2;
                // Complex z0conj = root0*(-1 + i*sqrt(3))/2;
                return;
            }

            // At this time, c0 != 0 and c1 != 0.
            Rational const rat2 = 2, rat3 = 3, rat4 = 4, rat27 = 27, rat108 = 108;
            Rational delta = -(rat4 * c1 * c1 * c1 + rat27 * c0 * c0);
            if (delta > zero)
            {
                // Three simple roots.
                Rational deltaDiv108 = delta / rat108;
                Rational betaRe = -c0 / rat2;
                Rational betaIm = std::sqrt(deltaDiv108);
                Rational theta = std::atan2(betaIm, betaRe);
                Rational thetaDiv3 = theta / rat3;
                double angle = (double)thetaDiv3;
                Rational cs = (Rational)std::cos(angle);
                Rational sn = (Rational)std::sin(angle);
                Rational rhoSqr = betaRe * betaRe + betaIm * betaIm;
                Rational rhoPowThird = (Rational)std::pow((double)rhoSqr, 1.0 / 6.0);
                Rational temp0 = rhoPowThird * cs;
                Rational temp1 = rhoPowThird * sn * (Rational)std::sqrt(3.0);
                Rational root0 = rat2 * temp0;
                Rational root1 = -temp0 - temp1;
                Rational root2 = -temp0 + temp1;
                rmMap.insert(std::make_pair(root0, 1));
                rmMap.insert(std::make_pair(root1, 1));
                rmMap.insert(std::make_pair(root2, 1));
                GTE_ROOTS_LOW_DEGREE_BLOCK(7);
            }
            else if (delta < zero)
            {
                // One simple root.
                Rational deltaDiv108 = delta / rat108;
                Rational temp0 = -c0 / rat2;
                Rational temp1 = (Rational)std::sqrt(-(double)deltaDiv108);
                Rational temp2 = temp0 - temp1;
                Rational temp3 = temp0 + temp1;
                if (temp2 >= zero)
                {
                    temp2 = (Rational)std::pow((double)temp2, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(8);
                }
                else
                {
                    temp2 = (Rational)-std::pow(-(double)temp2, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(9);
                }
                if (temp3 >= zero)
                {
                    temp3 = (Rational)std::pow((double)temp3, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(10);
                }
                else
                {
                    temp3 = (Rational)-std::pow(-(double)temp3, oneThird);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(11);
                }
                Rational root0 = temp2 + temp3;
                rmMap.insert(std::make_pair(root0, 1));

                // One complex conjugate pair.
                // Complex z0 = (-root0 - i*sqrt(3*root0*root0+4*c1))/2;
                // Complex z0conj = (-root0 + i*sqrt(3*root0*root0+4*c1))/2;
            }
            else  // delta = 0
            {
                // One simple root and one double root.
                Rational root0 = -rat3 * c0 / (rat2 * c1);
                Rational root1 = -rat2 * root0;
                rmMap.insert(std::make_pair(root0, 2));
                rmMap.insert(std::make_pair(root1, 1));
                GTE_ROOTS_LOW_DEGREE_BLOCK(12);
            }
        }

        template <typename Rational>
        static void SolveDepressedQuartic(Rational const& c0, Rational const& c1,
            Rational const& c2, std::map<Rational, int32_t>& rmMap)
        {
            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed cubic.
            Rational const zero = 0;
            if (c0 == zero)
            {
                SolveDepressedCubic(c1, c2, rmMap);
                auto iter = rmMap.find(zero);
                if (iter != rmMap.end())
                {
                    // The cubic has a root of zero, so the multiplicity must
                    // be increased.
                    ++iter->second;
                    GTE_ROOTS_LOW_DEGREE_BLOCK(13);
                }
                else
                {
                    // The cubic does not have a root of zero.  Insert the one
                    // for the quartic.
                    rmMap.insert(std::make_pair(zero, 1));
                    GTE_ROOTS_LOW_DEGREE_BLOCK(14);
                }
                return;
            }

            // Handle the special case of c1 = 0, in which case the quartic is
            // a biquadratic
            //   x^4 + c1*x^2 + c0 = (x^2 + c2/2)^2 + (c0 - c2^2/4)
            if (c1 == zero)
            {
                SolveBiquadratic(c0, c2, rmMap);
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

            if (delta > zero)
            {
                if (c2 < zero && a1 < zero)
                {
                    // Four simple real roots.
                    std::map<Real, int32_t> rmCubicMap;
                    SolveCubic(c1sqr - rat4 * c0 * c2, rat8 * c0, rat4 * c2, -rat8, rmCubicMap);
                    Rational t = (Rational)rmCubicMap.rbegin()->first;
                    Rational alphaSqr = rat2 * t - c2;
                    Rational alpha = (Rational)std::sqrt(std::max((double)alphaSqr, 0.0));
                    double sgnC1;
                    if (c1 > zero)
                    {
                        sgnC1 = 1.0;
                        GTE_ROOTS_LOW_DEGREE_BLOCK(15);
                    }
                    else
                    {
                        sgnC1 = -1.0;
                        GTE_ROOTS_LOW_DEGREE_BLOCK(16);
                    }
                    Rational arg = t * t - c0;
                    Rational beta = (Rational)(sgnC1 * std::sqrt(std::max((double)arg, 0.0)));
                    Rational D0 = alphaSqr - rat4 * (t + beta);
                    Rational sqrtD0 = (Rational)std::sqrt(std::max((double)D0, 0.0));
                    Rational D1 = alphaSqr - rat4 * (t - beta);
                    Rational sqrtD1 = (Rational)std::sqrt(std::max((double)D1, 0.0));
                    Rational root0 = (alpha - sqrtD0) / rat2;
                    Rational root1 = (alpha + sqrtD0) / rat2;
                    Rational root2 = (-alpha - sqrtD1) / rat2;
                    Rational root3 = (-alpha + sqrtD1) / rat2;
                    rmMap.insert(std::make_pair(root0, 1));
                    rmMap.insert(std::make_pair(root1, 1));
                    rmMap.insert(std::make_pair(root2, 1));
                    rmMap.insert(std::make_pair(root3, 1));
                }
                else // c2 >= 0 or a1 >= 0
                {
                    // Two complex-conjugate pairs.  The values alpha, D0
                    // and D1 are those of the if-block.
                    // Complex z0 = (alpha - i*sqrt(-D0))/2;
                    // Complex z0conj = (alpha + i*sqrt(-D0))/2;
                    // Complex z1 = (-alpha - i*sqrt(-D1))/2;
                    // Complex z1conj = (-alpha + i*sqrt(-D1))/2;
                    GTE_ROOTS_LOW_DEGREE_BLOCK(17);
                }
            }
            else if (delta < zero)
            {
                // Two simple real roots, one complex-conjugate pair.
                std::map<Real, int32_t> rmCubicMap;
                SolveCubic(c1sqr - rat4 * c0 * c2, rat8 * c0, rat4 * c2, -rat8,
                    rmCubicMap);
                Rational t = (Rational)rmCubicMap.rbegin()->first;
                Rational alphaSqr = rat2 * t - c2;
                Rational alpha = (Rational)std::sqrt(std::max((double)alphaSqr, 0.0));
                double sgnC1;
                if (c1 > zero)
                {
                    sgnC1 = 1.0;  // Leads to BLOCK(18)
                }
                else
                {
                    sgnC1 = -1.0;  // Leads to BLOCK(19)
                }
                Rational arg = t * t - c0;
                Rational beta = (Rational)(sgnC1 * std::sqrt(std::max((double)arg, 0.0)));
                Rational root0, root1;
                if (sgnC1 > 0.0)
                {
                    Rational D1 = alphaSqr - rat4 * (t - beta);
                    Rational sqrtD1 = (Rational)std::sqrt(std::max((double)D1, 0.0));
                    root0 = (-alpha - sqrtD1) / rat2;
                    root1 = (-alpha + sqrtD1) / rat2;

                    // One complex conjugate pair.
                    // Complex z0 = (alpha - i*sqrt(-D0))/2;
                    // Complex z0conj = (alpha + i*sqrt(-D0))/2;
                    GTE_ROOTS_LOW_DEGREE_BLOCK(18);
                }
                else
                {
                    Rational D0 = alphaSqr - rat4 * (t + beta);
                    Rational sqrtD0 = (Rational)std::sqrt(std::max((double)D0, 0.0));
                    root0 = (alpha - sqrtD0) / rat2;
                    root1 = (alpha + sqrtD0) / rat2;

                    // One complex conjugate pair.
                    // Complex z0 = (-alpha - i*sqrt(-D1))/2;
                    // Complex z0conj = (-alpha + i*sqrt(-D1))/2;
                    GTE_ROOTS_LOW_DEGREE_BLOCK(19);
                }
                rmMap.insert(std::make_pair(root0, 1));
                rmMap.insert(std::make_pair(root1, 1));
            }
            else  // delta = 0
            {
                if (a1 > zero || (c2 > zero && (a1 != zero || c1 != zero)))
                {
                    // One double real root, one complex-conjugate pair.
                    Rational const rat9 = 9;
                    Rational root0 = -c1 * a0 / (rat9 * c1sqr - rat2 * c2 * a1);
                    rmMap.insert(std::make_pair(root0, 2));

                    // One complex conjugate pair.
                    // Complex z0 = -root0 - i*sqrt(c2 + root0^2);
                    // Complex z0conj = -root0 + i*sqrt(c2 + root0^2);
                    GTE_ROOTS_LOW_DEGREE_BLOCK(20);
                }
                else
                {
                    Rational const rat3 = 3;
                    if (a0 != zero)
                    {
                        // One double real root, two simple real roots.
                        Rational const rat9 = 9;
                        Rational root0 = -c1 * a0 / (rat9 * c1sqr - rat2 * c2 * a1);
                        Rational alpha = rat2 * root0;
                        Rational beta = c2 + rat3 * root0 * root0;
                        Rational discr = alpha * alpha - rat4 * beta;
                        Rational temp1 = (Rational)std::sqrt(std::max((double)discr, 0.0));
                        Rational root1 = (-alpha - temp1) / rat2;
                        Rational root2 = (-alpha + temp1) / rat2;
                        rmMap.insert(std::make_pair(root0, 2));
                        rmMap.insert(std::make_pair(root1, 1));
                        rmMap.insert(std::make_pair(root2, 1));
                        GTE_ROOTS_LOW_DEGREE_BLOCK(21);
                    }
                    else
                    {
                        // One triple real root, one simple real root.
                        Rational root0 = -rat3 * c1 / (rat4 * c2);
                        Rational root1 = -rat3 * root0;
                        rmMap.insert(std::make_pair(root0, 3));
                        rmMap.insert(std::make_pair(root1, 1));
                        GTE_ROOTS_LOW_DEGREE_BLOCK(22);
                    }
                }
            }
        }

        template <typename Rational>
        static void SolveBiquadratic(Rational const& c0, Rational const& c2,
            std::map<Rational, int32_t>& rmMap)
        {
            // Solve x^4 + c2*x^2 + c0 = 0. We know that c0 != 0 at the time
            // of the SolveBiquadratic call, so x = 0 is not a root. Define
            // u = -c2/2 and v = c2^2/4 - c0 = u^2 - c0. Using the quadratic
            // formula,
            //   x^2 is in { u-sqrt(v), u+sqrt(v) }
            // Computing the square root,
            //   x is in { -sqrt(u-sqrt(v)), sqrt(u-sqrt(v)),
            //             -sqrt(u+sqrt(v)), sqrt(u+sqrt(v)) }
            // Because we know c0 != 0, which implies 0 is not a root, it
            // must be that u-sqrt(v) != 0 and u+sqrt(v) != 0.
            // 
            // Let z = a+b*i with b != 0. The square roots are +/- (c+d*i)
            // where
            //   c = sqrt((a+sqrt(a^2+b^2))/2)
            //   d = sign(b) * sqrt((-a+sqrt(a^2+b^2))/2)
            // 
            // v > 0, u-sqrt(v) > 0 [implies u+sqrt(v) > 0]: (block 23)
            //   Four real roots: r0, -r0, r1, -r1
            //     r0 = sqrt(u-sqrt(v))
            //     r1 = sqrt(u+sqrt(v))
            // 
            // v > 0, u+sqrt(v) < 0 [implies u-sqrt(v) < 0]: (block 24)
            //   Two complex conjugate pairs: z0, conj(z0), -z1, -conj(z1)
            //     z0 = sqrt(-u+sqrt(v)) * i
            //     z1 = sqrt(-u-sqrt(v)) * i
            // 
            // v > 0, u-sqrt(v) < 0, u+sqrt(v) > 0: (block 25)
            //   Two real roots, one complex conjugate pair: r0, -r0, z0, conj(z0)
            //    r0 = sqrt(u+sqrt(v))
            //    z0 = sqrt(-u+sqrt(v)) * i
            //
            // v < 0: (block 26)
            //   Two complex conjugate pairs: z0, conj(z0), -z0, -conj(z0)
            //     z0 = sqrt((u+sqrt(u^2-v))/2) - sqrt((-u+sqrt(u^2-v))/2) * i
            //        = sqrt((-c2/2+sqrt(c0))/2) - sqrt((c2/2+sqrt(c0))/2) * i
            // 
            // v = 0, u > 0: (block 27)
            //   Two real roots, each of multiplicity 2: r0, -r0
            //     r0 = sqrt(u) = sqrt(-c2/2)
            //
            // v = 0, u < 0: (block 28)
            //   Two complex conjugate pairs: z0, conj(z0), -z0, -conj(z0)
            //     z0 = sqrt(-u) * i = sqrt(c2/2) * i

            Rational const zero = static_cast<Rational>(0);
            Rational u = c2 / static_cast<Rational>(-2);
            Rational v = u * u - c0;
            if (v > zero)
            {
                Rational sqrtv = std::sqrt(v);
                Rational upsqrtv = u + sqrtv;
                // Compute u - sqrt(v) = c0 / (u + sqrt(v)) to avoid
                // subtractive cancellation.
                Rational umsqrtv = c0 / upsqrtv;
                if (umsqrtv > zero)
                {
                    // Real roots: r0, -r0, r1, -r1
                    // r0 = sqrt(u-sqrt(v))
                    // r1 = sqrt(u+sqrt(v))
                    Rational r0 = std::sqrt(umsqrtv);
                    Rational r1 = std::sqrt(upsqrtv);
                    rmMap.insert(std::make_pair(r0, 1));
                    rmMap.insert(std::make_pair(-r0, 1));
                    rmMap.insert(std::make_pair(r1, 1));
                    rmMap.insert(std::make_pair(-r1, 1));
                    GTE_ROOTS_LOW_DEGREE_BLOCK(23);
                }
                else if (upsqrtv < zero)
                {
                    // Complex roots: z0, conj(z0), -z1, -conj(z1)
                    // z0 = sqrt(-u+sqrt(v)) * i
                    // z1 = sqrt(-u-sqrt(v)) * i
                    GTE_ROOTS_LOW_DEGREE_BLOCK(24);
                }
                else  // umsqrtv < 0 and upsqrtv > 0
                {
                    // Real roots: r0, -r0
                    // Complex roots: z0, conj(z0)
                    // r0 = sqrt(u+sqrt(v))
                    // z0 = sqrt(-u+sqrt(v)) * i
                    Rational r0 = std::sqrt(upsqrtv);
                    rmMap.insert(std::make_pair(r0, 1));
                    rmMap.insert(std::make_pair(-r0, 1));
                    GTE_ROOTS_LOW_DEGREE_BLOCK(25);
                }
            }
            else if (v < zero)
            {
                // Complex roots: z0, conj(z0), -z0, -conj(z0)
                // z0 = sqrt((u+sqrt(u^2-v))/2)
                //      - sqrt((-u+sqrt(u^2-v))/2) * i
                GTE_ROOTS_LOW_DEGREE_BLOCK(26);
            }
            else // v = 0
            {
                if (u > zero)
                {
                    // Real roots: r0, r0, -r0, -r0
                    // r0 = sqrt(u)
                    Rational r0 = std::sqrt(u);
                    rmMap.insert(std::make_pair(r0, 2));
                    rmMap.insert(std::make_pair(-r0, 2));
                    GTE_ROOTS_LOW_DEGREE_BLOCK(27);
                }
                else  // u < 0
                {
                    // Complex roots: z0, conj(z0), z0, conj(z0)
                    // z0 = sqrt(-u) * i
                    GTE_ROOTS_LOW_DEGREE_BLOCK(28);
                }
            }
        }

        // Support for the GetNumRoots* functions.
        template <typename Rational>
        static void GetRootInfoDepressedQuadratic(Rational const& c0,
            std::vector<int32_t>& info)
        {
            Rational const zero = 0;
            if (c0 < zero)
            {
                // Two simple roots.
                info.push_back(1);
                info.push_back(1);
            }
            else if (c0 == zero)
            {
                // One double root.
                info.push_back(2);  // root is zero
            }
            else  // c0 > 0
            {
                // A complex-conjugate pair of roots.
            }
        }

        template <typename Rational>
        static void GetRootInfoDepressedCubic(Rational const& c0,
            Rational const& c1, std::vector<int32_t>& info)
        {
            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed quadratic.
            Rational const zero = 0;
            if (c0 == zero)
            {
                if (c1 == zero)
                {
                    info.push_back(3);  // triple root of zero
                }
                else
                {
                    info.push_back(1);  // simple root of zero
                    GetRootInfoDepressedQuadratic(c1, info);
                }
                return;
            }

            Rational const rat4 = 4, rat27 = 27;
            Rational delta = -(rat4 * c1 * c1 * c1 + rat27 * c0 * c0);
            if (delta > zero)
            {
                // Three simple real roots.
                info.push_back(1);
                info.push_back(1);
                info.push_back(1);
            }
            else if (delta < zero)
            {
                // One simple real root.
                info.push_back(1);
            }
            else  // delta = 0
            {
                // One simple real root and one double real root.
                info.push_back(1);
                info.push_back(2);
            }
        }

        template <typename Rational>
        static void GetRootInfoDepressedQuartic(Rational const& c0,
            Rational const& c1, Rational const& c2, std::vector<int32_t>& info)
        {
            // Handle the special case of c0 = 0, in which case the polynomial
            // reduces to a depressed cubic.
            Rational const zero = 0;
            if (c0 == zero)
            {
                if (c1 == zero)
                {
                    if (c2 == zero)
                    {
                        info.push_back(4);  // quadruple root of zero
                    }
                    else
                    {
                        info.push_back(2);  // double root of zero
                        GetRootInfoDepressedQuadratic(c2, info);
                    }
                }
                else
                {
                    info.push_back(1);  // simple root of zero
                    GetRootInfoDepressedCubic(c1, c2, info);
                }
                return;
            }

            // Handle the special case of c1 = 0, in which case the quartic is
            // a biquadratic
            //   x^4 + c1*x^2 + c0 = (x^2 + c2/2)^2 + (c0 - c2^2/4)
            if (c1 == zero)
            {
                GetRootInfoBiquadratic(c0, c2, info);
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

            if (delta > zero)
            {
                if (c2 < zero && a1 < zero)
                {
                    // Four simple real roots.
                    info.push_back(1);
                    info.push_back(1);
                    info.push_back(1);
                    info.push_back(1);
                }
                else // c2 >= 0 or a1 >= 0
                {
                    // Two complex-conjugate pairs.
                }
            }
            else if (delta < zero)
            {
                // Two simple real roots, one complex-conjugate pair.
                info.push_back(1);
                info.push_back(1);
            }
            else  // delta = 0
            {
                if (a1 > zero || (c2 > zero && (a1 != zero || c1 != zero)))
                {
                    // One double real root, one complex-conjugate pair.
                    info.push_back(2);
                }
                else
                {
                    if (a0 != zero)
                    {
                        // One double real root, two simple real roots.
                        info.push_back(2);
                        info.push_back(1);
                        info.push_back(1);
                    }
                    else
                    {
                        // One triple real root, one simple real root.
                        info.push_back(3);
                        info.push_back(1);
                    }
                }
            }
        }

        template <typename Rational>
        static void GetRootInfoBiquadratic(Rational const& c0,
            Rational const& c2, std::vector<int32_t>& info)
        {
            Rational const zero = static_cast<Rational>(0);
            Rational u = c2 / static_cast<Rational>(-2);
            Rational v = u * u - c0;
            if (v > zero)
            {
                Rational sqrtv = std::sqrt(v);
                Rational upsqrtv = u + sqrtv;
                Rational umsqrtv = c0 / upsqrtv;
                if (umsqrtv > zero)
                {
                    // Four simple roots.
                    info.push_back(1);
                    info.push_back(1);
                    info.push_back(1);
                    info.push_back(1);
                }
                else if (upsqrtv < zero)
                {
                    // Two simple complex conjugate pairs.
                }
                else  // umsqrtv < 0 and upsqrtv > 0
                {
                    // Two simple real roots, one complex conjugate pair.
                    info.push_back(1);
                    info.push_back(1);
                }
            }
            else if (v < zero)
            {
                // Two simple complex conjugate pairs.
            }
            else // v = 0
            {
                if (u > zero)
                {
                    // Two double real roots.
                    info.push_back(2);
                    info.push_back(2);
                }
                else  // u < 0
                {
                    // Double complex conjugate pairs.
                }
            }
        }

        // Support for the Find functions.
        static int32_t FindRecursive(int32_t degree, Real const* c, Real tmin, Real tmax,
            uint32_t maxIterations, Real* roots)
        {
            // The base of the recursion.
            Real const zero = (Real)0;
            Real root = zero;
            if (degree == 1)
            {
                int32_t numRoots;
                if (c[1] != zero)
                {
                    root = -c[0] / c[1];
                    numRoots = 1;
                }
                else if (c[0] == zero)
                {
                    root = zero;
                    numRoots = 1;
                }
                else
                {
                    numRoots = 0;
                }

                if (numRoots > 0 && tmin <= root && root <= tmax)
                {
                    roots[0] = root;
                    return 1;
                }
                return 0;
            }

            // Find the roots of the derivative polynomial scaled by 1/degree.
            // The scaling avoids the factorial growth in the coefficients;
            // for example, without the scaling, the high-order term x^d
            // becomes (d!)*x through multiple differentiations.  With the
            // scaling we instead get x.  This leads to better numerical
            // behavior of the root finder.
            int32_t derivDegree = degree - 1;
            std::vector<Real> derivCoeff(static_cast<size_t>(derivDegree) + 1);
            std::vector<Real> derivRoots(derivDegree);
            for (int32_t i = 0, ip1 = 1; i <= derivDegree; ++i, ++ip1)
            {
                derivCoeff[i] = c[ip1] * (Real)(ip1) / (Real)degree;
            }
            int32_t numDerivRoots = FindRecursive(degree - 1, &derivCoeff[0], tmin, tmax,
                maxIterations, &derivRoots[0]);

            int32_t numRoots = 0;
            if (numDerivRoots > 0)
            {
                // Find root on [tmin,derivRoots[0]].
                if (Find(degree, c, tmin, derivRoots[0], maxIterations, root))
                {
                    roots[numRoots++] = root;
                }

                // Find root on [derivRoots[i],derivRoots[i+1]].
                for (int32_t i = 0, ip1 = 1; i <= numDerivRoots - 2; ++i, ++ip1)
                {
                    if (Find(degree, c, derivRoots[i], derivRoots[ip1],
                        maxIterations, root))
                    {
                        roots[numRoots++] = root;
                    }
                }

                // Find root on [derivRoots[numDerivRoots-1],tmax].
                if (Find(degree, c, derivRoots[static_cast<size_t>(numDerivRoots) - 1], tmax,
                    maxIterations, root))
                {
                    roots[numRoots++] = root;
                }
            }
            else
            {
                // The polynomial is monotone on [tmin,tmax], so has at most one root.
                if (Find(degree, c, tmin, tmax, maxIterations, root))
                {
                    roots[numRoots++] = root;
                }
            }
            return numRoots;
        }

        static Real Evaluate(int32_t degree, Real const* c, Real t)
        {
            int32_t i = degree;
            Real result = c[i];
            while (--i >= 0)
            {
                result = t * result + c[i];
            }
            return result;
        }
    };
}
