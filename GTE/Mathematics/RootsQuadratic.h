// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.05.25

#pragma once

#include <Mathematics/APConversion.h>
#include <Mathematics/SWInterval.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

// Compute the real-valued roots of a quadratic polynomial with
// real-valued coefficients. The classification of roots and
// multiplicities is performed using a blend of interval arithmetic and
// rational arithmetic for exactness. For algorithmic details, see
// https://www.geometrictools.com/Documentation/LowDegreePolynomialRoots.pdf
//
// The general quadratic polynomial is p0 + p1 * x + p2 * x^2, where p2 is not
// zero. The monic quadratic polynomial is p0 + p1 * x + x^2. The depressed
// quadratic polynomial is p0 + x^2.
//
// TODO: For the solvers with rational inputs, determine the precision and
// maximum number of iterations internally. This will be based on known
// measures of root separation of polynomial roots. See the "Root separation"
// section of
// https://en.wikipedia.org/wiki/Geometrical_properties_of_polynomial_roots

namespace gte
{
    template <typename T>
    struct PolynomialRoot
    {
        PolynomialRoot()
            :
            x(static_cast<T>(0)),
            f(static_cast<T>(0)),
            m(0)
        {
        }

        PolynomialRoot(T const& inX, T const& inF, size_t inM)
            :
            x(inX),
            f(inF),
            m(inM)
        {
        }

        // x is the root (or root estimate), f is the polynomial evaluated
        // at x, and m specifies the multiplicity of x. The PolynomialRoot
        // object is invalid when m = 0.
        T x;
        T f;
        size_t m;
    };

    class RootsQuadratic
    {
    public:
        // Solve for the roots using floating-point arithmetic. The returned
        // size_t is the number of valid roots in the 'roots' array.

        // Solve the general quadratic p0 + x * (p1 + x * p2) = 0.
        template <typename T>
        static size_t Solve(T p0, T p1, T p2, std::array<PolynomialRoot<T>, 2>& roots)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            // Assign invalid numbers; the multiplicities are set to 0.
            T const zero = static_cast<T>(0);
            roots[0] = PolynomialRoot<T>{};
            roots[1] = PolynomialRoot<T>{};

            T xRoot{}, fRoot{};

            // Test whether the degree is smaller than 2.
            if (p2 == zero)
            {
                if (p1 == zero)
                {
                    // The solution set is either all real-valued x (p0 = 0)
                    // or no solution (p0 != 0). In either case, report no
                    // roots.
                    return 0;
                }
                else
                {
                    xRoot = -p0 / p1;
                    fRoot = p0 + xRoot * p1;
                    roots[0] = PolynomialRoot<T>(xRoot, fRoot, 1);
                    return 1;
                }
            }

            // Ensure the high-order coefficient is positive. The remaining
            // logic depends on p2 > 0.
            if (p2 < zero)
            {
                p0 = -p0;
                p1 = -p1;
                p2 = -p2;
            }

            // Test for zero-valued roots.
            if (p0 == zero)
            {
                if (p1 == zero)
                {
                    roots[0] = PolynomialRoot<T>(zero, zero, 2);
                    return 1;
                }
                else
                {
                    // Theoretically, F(x) = 0, but rounding errors in the
                    // computations can lead to F(x) not zero. Return the
                    // numerically computed function value.
                    xRoot = -p1 / p2;
                    fRoot = xRoot * (p1 + xRoot * p2);
                    if (xRoot > zero)
                    {
                        roots[0] = PolynomialRoot<T>(zero, zero, 1);
                        roots[1] = PolynomialRoot<T>(xRoot, fRoot, 1);
                    }
                    else
                    {
                        roots[0] = PolynomialRoot<T>(xRoot, fRoot, 1);
                        roots[1] = PolynomialRoot<T>(zero, zero, 1);
                    }
                    return 2;
                }
            }

            // Using rational arithmetic, correctly classify the roots.
            Number nDiscr{};
            Classification type = GetType(p0, p1, p2, nDiscr);
            if (type == Classification::Z0M1_Z0CONJM1)
            {
                return 0;
            }

            // Use floating-point arithmetic to compute real roots. Root
            // polishing is used to obtain accurate results and is less
            // expensive than rational arithmetic.
            if (type == Classification::R0M1_R1M1)
            {
                // The number nDiscr is guaranteed to be positive. The IEEE
                // 754-2019 Floating Point Standard requires std::sqrt to
                // return the floating-point number closest to the theoretical
                // value (using the current rounding mode). The interval
                // iSqrtDiscr is guaranteed to bound the theoretical square
                // root of the discriminant. The interval iNegP1Div2 is
                // guaranteed to bound the theoretical value of -p1/2.
                T constexpr tmax = std::numeric_limits<T>::max();
                SWInterval<T> iNegP1Div2 = SWInterval<T>::Mul(static_cast<T>(-0.5), p1);
                T discr = static_cast<T>(nDiscr);
                SWInterval<T> iSqrtDiscr(
                    std::nextafter(std::sqrt(discr), -tmax),
                    std::nextafter(std::sqrt(discr), +tmax));

                // The theoretical roots are x0 and x1 with x0 < x1. The
                // interval iX0 contains x0 and the interval iX1 contains x1.
                // The logic associated with the branch on sign of p1 is
                // designed to avoid subtractive cancellation.
                SWInterval<T> iTemp{}, iX0{}, iX1{};
                if (p1 > zero)
                {
                    iTemp = iNegP1Div2 - iSqrtDiscr;
                    iX0 = iTemp / p2;
                    iX1 = p0 / iTemp;
                }
                else
                {
                    iTemp = iNegP1Div2 + iSqrtDiscr;
                    iX0 = p0 / iTemp;
                    iX1 = iTemp / p2;
                }

                roots[0] = EstimateMinRoot(p0, p1, p2, iX0);
                roots[1] = EstimateMaxRoot(p0, p1, p2, iX1);
                return 2;
            }
            else // type == Classification::R0M2
            {
                xRoot = static_cast<T>(-0.5) * p1 / p2;
                fRoot = p0 + xRoot * (p1 + xRoot * p2);
                roots[0] = PolynomialRoot<T>(xRoot, fRoot, 2);
                return 1;
            }
        }

        // Solve the monic quadratic p0 + x * (p1 + x) = 0.
        template <typename T>
        static size_t Solve(T p0, T p1, std::array<PolynomialRoot<T>, 2>& roots)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            // Assign invalid numbers; the multiplicities are set to 0.
            T const zero = static_cast<T>(0);
            roots[0] = PolynomialRoot<T>{};
            roots[1] = PolynomialRoot<T>{};

            T xRoot{}, fRoot{};

            // Test for zero-valued roots.
            if (p0 == zero)
            {
                if (p1 == zero)
                {
                    roots[0] = PolynomialRoot<T>(zero, zero, 2);
                    return 1;
                }
                else
                {
                    // Theoretically, F(x) = 0, but rounding errors in the
                    // computations can lead to F(x) not zero. Return the
                    // numerically computed function value.
                    xRoot = -p1;
                    fRoot = xRoot * (p1 + xRoot);
                    if (xRoot > zero)
                    {
                        roots[0] = PolynomialRoot<T>(zero, zero, 1);
                        roots[1] = PolynomialRoot<T>(xRoot, fRoot, 1);
                    }
                    else
                    {
                        roots[0] = PolynomialRoot<T>(xRoot, fRoot, 1);
                        roots[1] = PolynomialRoot<T>(zero, zero, 1);
                    }
                    return 2;
                }
            }

            // Using rational arithmetic, correctly classify the roots.
            Number nDiscr{};
            Classification type = GetType(p0, p1, nDiscr);
            if (type == Classification::Z0M1_Z0CONJM1)
            {
                return 0;
            }

            // Use floating-point arithmetic to compute real roots. Root
            // polishing is used to obtain accurate results and is less
            // expensive than rational arithmetic.
            if (type == Classification::R0M1_R1M1)
            {
                // The number nDiscr is guaranteed to be positive. The IEEE
                // 754-2019 Floating Point Standard requires std::sqrt to
                // return the floating-point number closest to the theoretical
                // value (using the current rounding mode). The interval
                // iSqrtDiscr is guaranteed to bound the theoretical square
                // root of the discriminant. The interval iNegP1Div2 is
                // guaranteed to bound the theoretical value of -p1/2.
                T constexpr tmax = std::numeric_limits<T>::max();
                SWInterval<T> iNegP1Div2 = SWInterval<T>::Mul(static_cast<T>(-0.5), p1);
                T discr = static_cast<T>(nDiscr);
                SWInterval<T> iSqrtDiscr(
                    std::nextafter(std::sqrt(discr), -tmax),
                    std::nextafter(std::sqrt(discr), +tmax));

                // The theoretical roots are x0 and x1 with x0 < x1. The
                // interval iX0 contains x0 and the interval iX1 contains x1.
                // The logic associated with the branch on sign of p1 is
                // designed to avoid subtractive cancellation.
                SWInterval<T> iX0{}, iX1{};
                if (p1 > zero)
                {
                    iX0 = iNegP1Div2 - iSqrtDiscr;
                    iX1 = p0 / iX0;
                }
                else
                {
                    iX1 = iNegP1Div2 + iSqrtDiscr;
                    iX0 = p0 / iX1;
                }

                roots[0] = EstimateMinRoot(p0, p1, iX0);
                roots[1] = EstimateMaxRoot(p0, p1, iX1);
                return 2;
            }
            else // type == Classification::R0M2
            {
                xRoot = static_cast<T>(-0.5) * p1;
                fRoot = p0 + xRoot * (p1 + xRoot);
                roots[0] = PolynomialRoot<T>(xRoot, fRoot, 2);
                return 1;
            }
        }

        // Solve the depressed quadratic p0 + x * x = 0.
        template <typename T>
        static size_t Solve(T p0, std::array<PolynomialRoot<T>, 2>& roots)
        {
            static_assert(std::is_floating_point<T>::value,
                "The input type must be 'float' or 'double'.");

            T const zero = static_cast<T>(0);

            if (p0 < zero)
            {
                T xRoot = std::sqrt(-p0);
                T fRoot = p0 + xRoot * xRoot;
                roots[0] = PolynomialRoot<T>(-xRoot, fRoot, 1);
                roots[1] = PolynomialRoot<T>(+xRoot, fRoot, 1);
                return 2;
            }
            else if (p0 > zero)
            {
                roots[0] = PolynomialRoot<T>{};
                roots[1] = PolynomialRoot<T>{};
                return 0;
            }
            else
            {
                roots[0] = PolynomialRoot<T>(zero, zero, 2);
                roots[1] = PolynomialRoot<T>{};
                return 1;
            }
        }

        // Solve for the roots using floating-point arithmetic. The returned
        // size_t is the number of valid roots in the 'roots' array.
        using Rational = BSRational<UIntegerAP32>;

        static size_t Solve(int32_t precision, uint32_t maxIterations,
            Rational p0, Rational p1, Rational p2,
            std::array<PolynomialRoot<Rational>, 2>& roots)
        {
            // Assign invalid numbers; the multiplicities are set to 0.
            Rational const zero(0);
            roots[0] = PolynomialRoot<Rational>{};
            roots[1] = PolynomialRoot<Rational>{};

            // Test whether the degree is smaller than 2.
            if (p2.GetSign() == 0)
            {
                if (p1.GetSign() == 0)
                {
                    // The solution set is either all real-valued x (p0 = 0)
                    // or no solution (p0 != 0). In either case, report no
                    // roots.
                    return 0;
                }
                else
                {
                    Rational xRoot = -p0 / p1;
                    roots[0] = PolynomialRoot<Rational>(xRoot, zero, 1);
                    return 1;
                }
            }

            // Ensure the high-order coefficient is positive. The remaining
            // logic depends on p2 > 0.
            if (p2.GetSign() < 0)
            {
                p0.Negate();
                p1.Negate();
                p2.Negate();
            }

            // Test for zero-valued roots.
            if (p0.GetSign() == 0)
            {
                if (p1.GetSign() == 0)
                {
                    roots[0] = PolynomialRoot<Rational>(zero, zero, 2);
                    return 1;
                }
                else
                {
                    // Theoretically, F(x) = 0, but rounding errors in the
                    // computations can lead to F(x) not zero. Return the
                    // numerically computed function value.
                    Rational xRoot = -p1 / p2;
                    if (xRoot > zero)
                    {
                        roots[0] = PolynomialRoot<Rational>(zero, zero, 1);
                        roots[1] = PolynomialRoot<Rational>(xRoot, zero, 1);
                    }
                    else
                    {
                        roots[0] = PolynomialRoot<Rational>(xRoot, zero, 1);
                        roots[1] = PolynomialRoot<Rational>(zero, zero, 1);
                    }
                    return 2;
                }
            }

            // Using rational arithmetic, correctly classify the roots.
            Rational negP1Div2, discr{};
            Classification type = GetType(p0, p1, p2, negP1Div2, discr);
            if (type == Classification::Z0M1_Z0CONJM1)
            {
                return 0;
            }

            // Use floating-point arithmetic to compute real roots. Root
            // polishing is used to obtain accurate results and is less
            // expensive than rational arithmetic.
            if (type == Classification::R0M1_R1M1)
            {
                // The number nDiscr is guaranteed to be positive. The IEEE
                // 754-2019 Floating Point Standard requires std::sqrt to
                // return the floating-point number closest to the theoretical
                // value (using the current rounding mode). The interval
                // iSqrtDiscr is guaranteed to bound the theoretical square
                // root of the discriminant. The interval iNegP1Div2 is
                // guaranteed to bound the theoretical value of -p1/2.
                APConversion<Rational> apc(precision, maxIterations);
                Rational sqrtDiscr{};
                (void)apc.EstimateSqrt(discr, sqrtDiscr);

                // The theoretical roots are x0 and x1 with x0 < x1. The
                // sqrtDiscr is only an estimate of sqrt(discr) with the
                // user-specified precision.
                Rational temp{}, x0{}, x1{};
                if (p1 > zero)
                {
                    temp = negP1Div2 - sqrtDiscr;
                    x0 = temp / p2;
                    x1 = p0 / temp;
                }
                else
                {
                    temp = negP1Div2 + sqrtDiscr;
                    x0 = p0 / temp;
                    x1 = temp / p2;
                }

                Rational f0 = p0 + x0 * (p1 + x0 * p2);
                Rational f1 = p0 + x1 * (p1 + x1 * p2);
                roots[0] = PolynomialRoot<Rational>(x0, f0, 1);
                roots[1] = PolynomialRoot<Rational>(x1, f1, 1);
                return 2;
            }
            else // type == Classification::R0M2
            {
                Rational xRoot = negP1Div2 / p2;
                Rational fRoot = -discr / p2;
                roots[0] = PolynomialRoot<Rational>(xRoot, fRoot, 2);
                return 1;
            }
        }

        static size_t Solve(int32_t precision, uint32_t maxIterations,
            Rational p0, Rational p1, std::array<PolynomialRoot<Rational>, 2>& roots)
        {
            // Assign invalid numbers; the multiplicities are set to 0.
            Rational const zero(0);
            roots[0] = PolynomialRoot<Rational>{};
            roots[1] = PolynomialRoot<Rational>{};

            // Test for zero-valued roots.
            if (p0.GetSign() == 0)
            {
                if (p1.GetSign() == 0)
                {
                    roots[0] = PolynomialRoot<Rational>(zero, zero, 2);
                    return 1;
                }
                else
                {
                    // Theoretically, F(x) = 0, but rounding errors in the
                    // computations can lead to F(x) not zero. Return the
                    // numerically computed function value.
                    p1.Negate();
                    if (p1.GetSign() > 0)
                    {
                        roots[0] = PolynomialRoot<Rational>(zero, zero, 1);
                        roots[1] = PolynomialRoot<Rational>(p1, zero, 1);
                    }
                    else
                    {
                        roots[0] = PolynomialRoot<Rational>(p1, zero, 1);
                        roots[1] = PolynomialRoot<Rational>(zero, zero, 1);
                    }
                    return 2;
                }
            }

            // Using rational arithmetic, correctly classify the roots.
            Rational negP1Div2, discr{};
            Classification type = GetType(p0, p1, negP1Div2, discr);
            if (type == Classification::Z0M1_Z0CONJM1)
            {
                return 0;
            }

            // Use floating-point arithmetic to compute real roots. Root
            // polishing is used to obtain accurate results and is less
            // expensive than rational arithmetic.
            if (type == Classification::R0M1_R1M1)
            {
                // The number nDiscr is guaranteed to be positive. The IEEE
                // 754-2019 Floating Point Standard requires std::sqrt to
                // return the floating-point number closest to the theoretical
                // value (using the current rounding mode). The interval
                // iSqrtDiscr is guaranteed to bound the theoretical square
                // root of the discriminant. The interval iNegP1Div2 is
                // guaranteed to bound the theoretical value of -p1/2.
                APConversion<Rational> apc(precision, maxIterations);
                Rational sqrtDiscr{};
                (void)apc.EstimateSqrt(discr, sqrtDiscr);

                // The theoretical roots are x0 and x1 with x0 < x1. The
                // sqrtDiscr is only an estimate of sqrt(discr) with the
                // user-specified precision.
                Rational x0{}, x1{};
                if (p1 > zero)
                {
                    x0 = negP1Div2 - sqrtDiscr;
                    x1 = p0 / x0;
                }
                else
                {
                    x1 = negP1Div2 + sqrtDiscr;
                    x0 = p0 / x1;
                }

                Rational f0 = p0 + x0 * (p1 + x0);
                Rational f1 = p0 + x1 * (p1 + x1);
                roots[0] = PolynomialRoot<Rational>(x0, f0, 1);
                roots[1] = PolynomialRoot<Rational>(x1, f1, 1);
                return 2;
            }
            else // type == Classification::R0M2
            {
                discr.Negate();
                roots[0] = PolynomialRoot<Rational>(negP1Div2, discr, 2);
                return 1;
            }
        }

        // Solve the depressed quadratic p0 + x * x = 0.
        static size_t Solve(int32_t precision, uint32_t maxIterations,
            Rational p0, std::array<PolynomialRoot<Rational>, 2>& roots)
        {
            Rational discr = p0;
            discr.Negate();
            if (discr.GetSign() > 0)
            {
                APConversion<Rational> apc(precision, maxIterations);
                Rational sqrtDiscr{};
                (void)apc.EstimateSqrt(discr, sqrtDiscr);

                Rational fRoot = p0 + sqrtDiscr * sqrtDiscr;
                roots[0] = PolynomialRoot<Rational>(-sqrtDiscr, fRoot, 1);
                roots[1] = PolynomialRoot<Rational>(+sqrtDiscr, fRoot, 1);
                return 2;
            }
            else if (discr.GetSign() < 0)
            {
                roots[0] = PolynomialRoot<Rational>{};
                roots[1] = PolynomialRoot<Rational>{};
                return 0;
            }
            else
            {
                Rational zero(0);
                roots[0] = PolynomialRoot<Rational>(zero, zero, 2);
                roots[1] = PolynomialRoot<Rational>{};
                return 1;
            }
        }

    private:
        // The root classifications are guaranteed to be theoretically
        // correct assuming the floating-point coefficients are error
        // free. The computations use rational arithmetic.
        using Number = BSNumber<UIntegerAP32>;

        enum class Classification
        {
            // Default value for construction.
            INVALID,

            // Two real roots, each multiplicity 1.
            R0M1_R1M1,

            // One real root of multiplicity 2.
            R0M2,

            // A pair of complex conjugate roots, each multiplicity 1.
            Z0M1_Z0CONJM1
        };

        // Support for the floating-point general quadratic.
        template <typename T>
        static Classification GetType(T p0, T p1, T p2, Number& nDiscr)
        {
            Number nP0(p0), nP1(p1), nP2(p2), nP1Div2(std::ldexp(nP1, -1));
            nDiscr = nP1Div2 * nP1Div2 - nP0 * nP2;
            int32_t sign = nDiscr.GetSign();
            if (sign > 0)
            {
                return Classification::R0M1_R1M1;
            }
            else if (sign < 0)
            {
                return Classification::Z0M1_Z0CONJM1;
            }
            else
            {
                return Classification::R0M2;
            }
        }

        template <typename T>
        static PolynomialRoot<T> EstimateMinRoot(T p0, T p1, T p2, SWInterval<T> const& iX)
        {
            T const zero = static_cast<T>(0);
            T xRoot{}, fRoot{};

            T fMin = p0 + iX[0] * (p1 + iX[0] * p2);
            if (fMin > zero)
            {
                T fMax = p0 + iX[1] * (p1 + iX[1] * p2);
                if (fMax < zero)
                {
                    // Use bisection to estimate the root.
                    Bisect(p0, p1, p2, iX[0], iX[1], +1, xRoot, fRoot);
                }
                else if (fMax == zero)
                {
                    // The floating-point root estimate is iX[1].
                    xRoot = iX[1];
                    fRoot = zero;
                }
                else // fMax > 0
                {
                    // Both theoretical roots are in the iX interval. The
                    // polynomial value at the local minimum point -p1/(2*p2)
                    // is theoretically negative, so attempt to use this for
                    // root bounding.
                    T xMid = static_cast<T>(-0.5) * p1 / p2;
                    T fMid = p0 + xMid * (p1 + xMid * p2);
                    if (fMid < zero)
                    {
                        // Use bisection to estimate the root.
                        Bisect(p0, p1, p2, iX[0], xMid, +1, xRoot, fRoot);
                    }
                    else // fMid >= 0
                    {
                        // Floating-point rounding errors have occurred. Use
                        // xMid as the root estimate.
                        xRoot = xMid;
                        fRoot = fMid;
                    }
                }
            }
            else  // fMin = 0
            {
                // Theoretically, fmin > 0 but floating-point rounding errors
                // have occurred. Use iX[0] as the root estimate.
                xRoot = iX[0];
                fRoot = fMin;
            }

            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        template <typename T>
        static PolynomialRoot<T> EstimateMaxRoot(T p0, T p1, T p2, SWInterval<T> const& iX)
        {
            T const zero = static_cast<T>(0);
            T xRoot{}, fRoot{};

            T fMax = p0 + iX[1] * (p1 + iX[1] * p2);
            if (fMax > zero)
            {
                T fMin = p0 + iX[0] * (p1 + iX[0] * p2);
                if (fMin < zero)
                {
                    // Use bisection to estimate the root.
                    Bisect(p0, p1, p2, iX[0], iX[1], -1, xRoot, fRoot);
                }
                else if (fMin == zero)
                {
                    // The floating-point root estimate is iX[0].
                    xRoot = iX[0];
                    fRoot = zero;
                }
                else // fMin > 0
                {
                    // Both theoretical roots are in the iX interval. The
                    // polynomial value at the local minimum point -p1/(2*p2)
                    // is theoretically negative, so attempt to use this for
                    // root bounding.
                    T xMid = static_cast<T>(-0.5) * p1 / p2;
                    T fMid = p0 + xMid * (p1 + xMid * p2);
                    if (fMid < zero)
                    {
                        // Use bisection to estimate the root.
                        Bisect(p0, p1, p2, xMid, iX[1], -1, xRoot, fRoot);
                    }
                    else // fMid >= 0
                    {
                        // Floating-point rounding errors have occurred. Use
                        // xMid as the root estimate.
                        xRoot = xMid;
                        fRoot = fMid;
                    }
                }
            }
            else
            {
                // Theoretically, fmax > 0 but floating-point rounding errors
                // have occurred. Use iX[1] as the root estimate.
                xRoot = iX[1];
                fRoot = fMax;
            }

            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        // The signFMin is +1 or -1. The signFMax is -signFMin, but because
        // signFMax is not used in the code explicitly, it does not need to
        // be passed to Bisect(...).
        template <typename T>
        static void Bisect(T p0, T p1, T p2, T xMin, T xMax, int32_t signFMin,
            T& xRoot, T& fAtXRoot)
        {
            // The bisection steps. The iteration algorithm terminates when
            // the midpoint of the current interval equals one of the interval
            // endpoints. At this time the interval endpoints are consecutive
            // floating-point numbers. TODO: Verify 4096 suffices for 'float'
            // and for 'double'.
            size_t constexpr maxIterations = 4096;
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T x0 = xMin, x1 = xMax;
            for (size_t iteration = 1; iteration < maxIterations; ++iteration)
            {
                xRoot = half * (x0 + x1);
                fAtXRoot = p0 + xRoot * (p1 + xRoot * p2);

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive floating-point numbers in
                    // which case subdivision cannot produce a floating-point
                    // number between them.
                    return;
                }

                int32_t signFAtXRoot = (fAtXRoot > zero ? +1 : (fAtXRoot < zero ? -1 : 0));
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    return;
                }

                // Update the correct endpoint to the midpoint.
                if (signFAtXRoot == signFMin)
                {
                    x0 = xRoot;
                }
                else // signFAtXRoot == signFMax)
                {
                    x1 = xRoot;
                }
            }

            // The maximum number of iterations has been exceeded. The last
            // computed values of xRoot and fAtXRoot are used as the polished
            // root.
        }

        // Support for the floating-point monic quadratic.
        template <typename T>
        static Classification GetType(T p0, T p1, Number& nDiscr)
        {
            Number nP0(p0), nP1(p1), nNegP1Div2(std::ldexp(nP1, -1));
            nDiscr = nNegP1Div2 * nNegP1Div2 - nP0;
            int32_t sign = nDiscr.GetSign();
            if (sign > 0)
            {
                return Classification::R0M1_R1M1;
            }
            else if (sign < 0)
            {
                return Classification::Z0M1_Z0CONJM1;
            }
            else
            {
                return Classification::R0M2;
            }
        }

        template <typename T>
        static PolynomialRoot<T> EstimateMinRoot(T p0, T p1, SWInterval<T> const& iX)
        {
            T const zero = static_cast<T>(0);
            T xRoot{}, fRoot{};

            T fMin = p0 + iX[0] * (p1 + iX[0]);
            if (fMin > zero)
            {
                T fMax = p0 + iX[1] * (p1 + iX[1]);
                if (fMax < zero)
                {
                    // Use bisection to estimate the root.
                    Bisect(p0, p1, iX[0], iX[1], +1, xRoot, fRoot);
                }
                else if (fMax == zero)
                {
                    // The floating-point root estimate is iX[1].
                    xRoot = iX[1];
                    fRoot = zero;
                }
                else // fMax > 0
                {
                    // Both theoretical roots are in the iX interval. The
                    // polynomial value at the local minimum point -p1/2
                    // is theoretically negative, so attempt to use this for
                    // root bounding.
                    T xMid = static_cast<T>(-0.5) * p1;
                    T fMid = p0 + xMid * (p1 + xMid);
                    if (fMid < zero)
                    {
                        // Use bisection to estimate the root.
                        Bisect(p0, p1, iX[0], xMid, +1, xRoot, fRoot);
                    }
                    else // fMid >= 0
                    {
                        // Floating-point rounding errors have occurred. Use
                        // xMid as the root estimate.
                        xRoot = xMid;
                        fRoot = fMid;
                    }
                }
            }
            else  // fMin = 0
            {
                // Theoretically, fmin > 0 but floating-point rounding errors
                // have occurred. Use iX[0] as the root estimate.
                xRoot = iX[0];
                fRoot = fMin;
            }

            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        template <typename T>
        static PolynomialRoot<T> EstimateMaxRoot(T p0, T p1, SWInterval<T> const& iX)
        {
            T const zero = static_cast<T>(0);
            T xRoot{}, fRoot{};

            T fMax = p0 + iX[1] * (p1 + iX[1]);
            if (fMax > zero)
            {
                T fMin = p0 + iX[0] * (p1 + iX[0]);
                if (fMin < zero)
                {
                    // Use bisection to estimate the root.
                    Bisect(p0, p1, iX[0], iX[1], -1, xRoot, fRoot);
                }
                else if (fMin == zero)
                {
                    // The floating-point root estimate is iX[0].
                    xRoot = iX[0];
                    fRoot = zero;
                }
                else // fMin > 0
                {
                    // Both theoretical roots are in the iX interval. The
                    // polynomial value at the local minimum point -p1/2
                    // is theoretically negative, so attempt to use this for
                    // root bounding.
                    T xMid = static_cast<T>(-0.5) * p1;
                    T fMid = p0 + xMid * (p1 + xMid);
                    if (fMid < zero)
                    {
                        // Use bisection to estimate the root.
                        Bisect(p0, p1, xMid, iX[1], -1, xRoot, fRoot);
                    }
                    else // fMid >= 0
                    {
                        // Floating-point rounding errors have occurred. Use
                        // xMid as the root estimate.
                        xRoot = xMid;
                        fRoot = fMid;
                    }
                }
            }
            else
            {
                // Theoretically, fmax > 0 but floating-point rounding errors
                // have occurred. Use iX[1] as the root estimate.
                xRoot = iX[1];
                fRoot = fMax;
            }

            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        // The signFMin is +1 or -1. The signFMax is -signFMin, but because
        // signFMax is not used in the code explicitly, it does not need to
        // be passed to Bisect(...).
        template <typename T>
        static void Bisect(T p0, T p1, T xMin, T xMax, int32_t signFMin,
            T& xRoot, T& fAtXRoot)
        {
            // The bisection steps. The iteration algorithm terminates when
            // the midpoint of the current interval equals one of the interval
            // endpoints. At this time the interval endpoints are consecutive
            // floating-point numbers. TODO: Verify 4096 suffices for 'float'
            // and for 'double'.
            size_t constexpr maxIterations = 4096;
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T x0 = xMin, x1 = xMax;
            for (size_t iteration = 1; iteration < maxIterations; ++iteration)
            {
                xRoot = half * (x0 + x1);
                fAtXRoot = p0 + xRoot * (p1 + xRoot);

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive floating-point numbers in
                    // which case subdivision cannot produce a floating-point
                    // number between them.
                    return;
                }

                int32_t signFAtXRoot = (fAtXRoot > zero ? +1 : (fAtXRoot < zero ? -1 : 0));
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    return;
                }

                // Update the correct endpoint to the midpoint.
                if (signFAtXRoot == signFMin)
                {
                    x0 = xRoot;
                }
                else // signFAtXRoot == signFMax)
                {
                    x1 = xRoot;
                }
            }

            // The maximum number of iterations has been exceeded. The last
            // computed values of xRoot and fAtXRoot are used as the polished
            // root.
        }

        // Support for the rational general quadratic.
        static Classification GetType(
            Rational const& p0, Rational const& p1, Rational const& p2,
            Rational& negP1Div2, Rational& discr)
        {
            negP1Div2 = std::ldexp(p1, -1);
            negP1Div2.Negate();
            discr = negP1Div2 * negP1Div2 - p0 * p2;
            int32_t sign = discr.GetSign();
            if (sign > 0)
            {
                return Classification::R0M1_R1M1;
            }
            else if (sign < 0)
            {
                return Classification::Z0M1_Z0CONJM1;
            }
            else
            {
                return Classification::R0M2;
            }
        }

        static Classification GetType(
            Rational const& p0, Rational const& p1,
            Rational& negP1Div2, Rational& discr)
        {
            negP1Div2 = std::ldexp(p1, -1);
            negP1Div2.Negate();
            discr = negP1Div2 * negP1Div2 - p0;
            int32_t sign = discr.GetSign();
            if (sign > 0)
            {
                return Classification::R0M1_R1M1;
            }
            else if (sign < 0)
            {
                return Classification::Z0M1_Z0CONJM1;
            }
            else
            {
                return Classification::R0M2;
            }
        }
    };
}
