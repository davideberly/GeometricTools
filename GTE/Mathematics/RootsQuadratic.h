// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.06.16

#pragma once

#include <Mathematics/APConversion.h>
#include <Mathematics/SWInterval.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
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

        PolynomialRoot(PolynomialRoot const& other) = default;

        PolynomialRoot(T const& inX, T const& inF, size_t inM)
            :
            x(inX),
            f(inF),
            m(inM)
        {
        }

        PolynomialRoot& operator=(PolynomialRoot const& other) = default;

        bool operator==(PolynomialRoot& other) const
        {
            return x == other.x;
        }

        bool operator<(PolynomialRoot& other) const
        {
            return x < other.x;
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
                    T xRoot = -p0 / p1;
                    T fRoot = p0 + xRoot * p1;
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
                    T xRoot = -p1 / p2;
                    T fRoot = xRoot * (p1 + xRoot * p2);
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
                VerifySqrtInterval(nDiscr, iSqrtDiscr);

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

                roots[0] = EstimateRootFDecreasing(p0, p1, p2, iX0[0], iX0[1]);
                roots[1] = EstimateRootFIncreasing(p0, p1, p2, iX1[0], iX1[1]);
                return 2;
            }
            else if (type == Classification::R0M2)
            {
                T xRoot = static_cast<T>(-0.5) * p1 / p2;
                T fRoot = p0 + xRoot * (p1 + xRoot * p2);
                roots[0] = PolynomialRoot<T>(xRoot, fRoot, 2);
                return 1;
            }
            else // type == Classification::Z0M1_Z0CONJM1
            {
                return 0;
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
                    T xRoot = -p1;
                    T fRoot = xRoot * (p1 + xRoot);
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
                VerifySqrtInterval(nDiscr, iSqrtDiscr);

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

                roots[0] = EstimateRootFDecreasing(p0, p1, iX0[0], iX0[1]);
                roots[1] = EstimateRootFIncreasing(p0, p1, iX1[0], iX1[1]);
                return 2;
            }
            else if (type == Classification::R0M2)
            {
                T xRoot = static_cast<T>(-0.5) * p1;
                T fRoot = p0 + xRoot * (p1 + xRoot);
                roots[0] = PolynomialRoot<T>(xRoot, fRoot, 2);
                return 1;
            }
            else // type == Classification::Z0M1_Z0CONJM1
            {
                return 0;
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
            else if (type == Classification::R0M2)
            {
                Rational xRoot = negP1Div2 / p2;
                Rational fRoot = -discr / p2;
                roots[0] = PolynomialRoot<Rational>(xRoot, fRoot, 2);
                return 1;
            }
            else // type == Classification::Z0M1_Z0CONJM1
            {
                return 0;
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
            else if (type == Classification::R0M2)
            {
                discr.Negate();
                roots[0] = PolynomialRoot<Rational>(negP1Div2, discr, 2);
                return 1;
            }
            else // type == Classification::Z0M1_Z0CONJM1
            {
                return 0;
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
        // TODO: Verify 4096 suffices for both 'float' and 'double'.
        static size_t constexpr maxBisections = 4096;

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
        static PolynomialRoot<T> EstimateRootFDecreasing(T p0, T p1, T p2, T xMin, T xMax)
        {
            T const zero = static_cast<T>(0);
            T fMin = std::fma(std::fma(p2, xMin, p1), xMin, p0);
            if (fMin > zero)
            {
                T fMax = std::fma(std::fma(p2, xMax, p1), xMax, p0);
                if (fMax < zero)
                {
                    // The signs agree with the theoretical values obtained by
                    // interval arithmetic performed by the caller. Use
                    // bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, p2, xMin, xMax, +1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                if (fMax == zero)
                {
                    // The floating-point root estimate is xMax.
                    return PolynomialRoot<T>(xMax, zero, 1);
                }

                // The value is fMax > 0. Both theoretical roots are in the
                // x-interval. The polynomial value at the local minimum point
                // -p1/(2*p2) is theoretically negative, so attempt to use
                // this for root bounding.
                T xMid = static_cast<T>(-0.5) * p1 / p2;
                T fMid = std::fma(std::fma(p2, xMid, p1), xMid, p0);
                if (fMid < zero)
                {
                    // Use bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, p2, xMin, xMid, +1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                // The value is fMid >= 0. Floating-point rounding errors have
                // occurred. Use rational arithmetic instead (fall-through to
                // last block of code).
            }

            // Floating-point rounding errors have occurred that affect the
            // signs. This occurs because the two distinct roots are nearly
            // equal. Perform the bisection using rational arithmetic with
            // precision that of type T (24 for 'float' and 53 for 'double').
            Rational const rNegHalf(-0.5);
            Rational rP0(p0), rP1(p1), rP2(p2);
            Rational rXMin(xMin), rXMid = rNegHalf * rP1 / rP2;
            Rational rFMin = rP0 + rXMin * (rP1 + rXMin * rP2);
            Rational rFMid = rP0 + rXMid * (rP1 + rXMid * rP2);
            LogAssert(
                rFMin.GetSign() > 0 && rFMid.GetSign() < 0,
                "Unexpected result, disagrees with the theory.");

            Rational rXRoot{}, rFRoot{};
            (void)Bisect(std::numeric_limits<T>::digits,
                rP0, rP1, rP2, rXMin, rXMid, +1, rXRoot, rFRoot);

            T xRoot = static_cast<T>(rXRoot);
            T fRoot = static_cast<T>(rFRoot);
            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        template <typename T>
        static PolynomialRoot<T> EstimateRootFIncreasing(T p0, T p1, T p2, T xMin, T xMax)
        {
            T const zero = static_cast<T>(0);
            T fMax = std::fma(std::fma(p2, xMax, p1), xMax, p0);
            if (fMax > zero)
            {
                T fMin = std::fma(std::fma(p2, xMin, p1), xMin, p0);
                if (fMin < zero)
                {
                    // The signs agree with the theoretical values obtained by
                    // interval arithmetic performed by the caller. Use
                    // bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, p2, xMin, xMax, -1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                if (fMin == zero)
                {
                    // The floating-point root estimate is xMin.
                    return PolynomialRoot<T>(xMin, zero, 1);
                }

                // The value is fMin > 0. Both theoretical roots are in the
                // x-interval. The polynomial value at the local minimum point
                // -p1/(2*p2) is theoretically negative, so attempt to use
                // this for root bounding.
                T xMid = static_cast<T>(-0.5) * p1 / p2;
                T fMid = std::fma(std::fma(p2, xMid, p1), xMid, p0);
                if (fMid < zero)
                {
                    // Use bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, p2, xMid, xMax, -1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                // The value is fMid >= 0. Floating-point rounding errors have
                // occurred. Use rational arithmetic instead (fall-through to
                // last block of code).
            }

            // Floating-point rounding errors have occurred that affect the
            // signs. This occurs because the two distinct roots are nearly
            // equal. Perform the bisection using rational arithmetic with
            // precision that of type T (24 for 'float' and 53 for 'double').
            Rational const rNegHalf(-0.5);
            Rational rP0(p0), rP1(p1), rP2(p2);
            Rational rXMax(xMax), rXMid = rNegHalf * rP1 / rP2;
            Rational rFMax = rP0 + rXMax * (rP1 + rXMax * rP2);
            Rational rFMid = rP0 + rXMid * (rP1 + rXMid * rP2);
            LogAssert(
                rFMax.GetSign() > 0 && rFMid.GetSign() < 0,
                "Unexpected result, disagrees with the theory.");

            Rational rXRoot{}, rFRoot{};
            (void)Bisect(std::numeric_limits<T>::digits,
                rP0, rP1, rP2, rXMid, rXMax, -1, rXRoot, rFRoot);

            T xRoot = static_cast<T>(rXRoot);
            T fRoot = static_cast<T>(rFRoot);
            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        // The signFMin is +1 or -1. The signFMax is -signFMin, but because
        // signFMax is not used in the code explicitly, it does not need to
        // be passed to Bisect(...).
        template <typename T>
        static size_t Bisect(T p0, T p1, T p2, T xMin, T xMax, int32_t signFMin,
            T& xRoot, T& fAtXRoot)
        {
            // The bisection steps. The iteration algorithm terminates when
            // the midpoint of the current interval equals one of the interval
            // endpoints. At this time the interval endpoints are consecutive
            // floating-point numbers.
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T x0 = xMin, x1 = xMax;
            size_t iteration{};
            for (iteration = 1; iteration < maxBisections; ++iteration)
            {
                xRoot = half * (x0 + x1);
                fAtXRoot = std::fma(std::fma(p2, xRoot, p1), xRoot, p0);

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive floating-point numbers in
                    // which case subdivision cannot produce a floating-point
                    // number between them.
                    break;
                }

                int32_t signFAtXRoot = (fAtXRoot > zero ? +1 : (fAtXRoot < zero ? -1 : 0));
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    break;
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
            return iteration;
        }

        static size_t Bisect(int32_t precision, Rational const& p0, Rational const& p1,
            Rational const& p2, Rational const& xMin, Rational const& xMax,
            int32_t signFMin, Rational& xRoot, Rational& fAtXRoot)
        {
            Rational x0 = xMin, x1 = xMax;
            size_t iteration{};
            for (iteration = 1; iteration < maxBisections; ++iteration)
            {
                Rational average = std::ldexp(x0 + x1, -1);  // (x0 + x1) / 2
                Convert(average, precision, FE_TONEAREST, xRoot);
                fAtXRoot = p0 + xRoot * (p1 + xRoot * p2);

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive rational numbers of the
                    // specified precision in which case subdivision cannot
                    // produce a rational number between them.
                    break;
                }

                int32_t signFAtXRoot = fAtXRoot.GetSign();
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    break;
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
            return iteration;
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
        static PolynomialRoot<T> EstimateRootFDecreasing(T p0, T p1, T xMin, T xMax)
        {
            T const zero = static_cast<T>(0);
            T fMin = RobustSumOfProducts(xMin, xMin, p1, xMin) + p0;
            if (fMin > zero)
            {
                T fMax = RobustSumOfProducts(xMax, xMax, p1, xMax) + p0;
                if (fMax < zero)
                {
                    // The signs agree with the theoretical values obtained by
                    // interval arithmetic performed by the caller. Use
                    // bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, xMin, xMax, +1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                if (fMax == zero)
                {
                    // The floating-point root estimate is xMax.
                    return PolynomialRoot<T>(xMax, zero, 1);
                }

                // The value is fMax > 0. Both theoretical roots are in the
                // x-interval. The polynomial value at the local minimum point
                // -p1/(2*p2) is theoretically negative, so attempt to use
                // this for root bounding.
                T xMid = static_cast<T>(-0.5) * p1;
                T fMid = RobustSumOfProducts(xMid, xMid, p1, xMid) + p0;
                if (fMid < zero)
                {
                    // Use bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, xMin, xMid, +1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                // The value is fMid >= 0. Floating-point rounding errors have
                // occurred. Use rational arithmetic instead (fall-through to
                // last block of code).
            }

            // Floating-point rounding errors have occurred that affect the
            // signs. This occurs because the two distinct roots are nearly
            // equal. Perform the bisection using rational arithmetic with
            // precision that of type T (24 for 'float' and 53 for 'double').
            Number const nNegHalf(-0.5);
            Number nP0(p0), nP1(p1);
            Number nXMin(xMin), nXMid = nNegHalf * nP1;
            Number nFMin = nP0 + nXMin * (nP1 + nXMin);
            Number nFMid = nP0 + nXMid * (nP1 + nXMid);
            LogAssert(
                nFMin.GetSign() > 0 && nFMid.GetSign() < 0,
                "Unexpected result, disagrees with the theory.");

            Number nXRoot{}, nFRoot{};
            (void)Bisect(std::numeric_limits<T>::digits,
                nP0, nP1, nXMin, nXMid, +1, nXRoot, nFRoot);

            T xRoot = static_cast<T>(nXRoot);
            T fRoot = static_cast<T>(nFRoot);
            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        template <typename T>
        static PolynomialRoot<T> EstimateRootFIncreasing(T p0, T p1, T xMin, T xMax)
        {
            T const zero = static_cast<T>(0);
            T fMax = RobustSumOfProducts(xMax, xMax, p1, xMax) + p0;
            if (fMax > zero)
            {
                T fMin = RobustSumOfProducts(xMin, xMin, p1, xMin) + p0;
                if (fMin < zero)
                {
                    // The signs agree with the theoretical values obtained by
                    // interval arithmetic performed by the caller. Use
                    // bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, xMin, xMax, -1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                if (fMin == zero)
                {
                    // The floating-point root estimate is xMin.
                    return PolynomialRoot<T>(xMin, zero, 1);
                }

                // The value is fMin > 0. Both theoretical roots are in the
                // x-interval. The polynomial value at the local minimum point
                // -p1/(2*p2) is theoretically negative, so attempt to use
                // this for root bounding.
                T xMid = static_cast<T>(-0.5) * p1;
                T fMid = RobustSumOfProducts(xMid, xMid, p1, xMid) + p0;
                if (fMid < zero)
                {
                    // Use bisection to estimate the root.
                    T xRoot{}, fRoot{};
                    (void)Bisect(p0, p1, xMid, xMax, -1, xRoot, fRoot);
                    return PolynomialRoot<T>(xRoot, fRoot, 1);
                }

                // The value is fMid >= 0. Floating-point rounding errors have
                // occurred. Use rational arithmetic instead (fall-through to
                // last block of code).
            }

            // Floating-point rounding errors have occurred that affect the
            // signs. This occurs because the two distinct roots are nearly
            // equal. Perform the bisection using rational arithmetic with
            // precision that of type T (24 for 'float' and 53 for 'double').
            Number const nNegHalf(-0.5);
            Number nP0(p0), nP1(p1);
            Number nXMax(xMax), nXMid = nNegHalf * nP1;
            Number nFMax = nP0 + nXMax * (nP1 + nXMax);
            Number nFMid = nP0 + nXMid * (nP1 + nXMid);
            LogAssert(
                nFMax.GetSign() > 0 && nFMid.GetSign() < 0,
                "Unexpected result, disagrees with the theory.");

            Number nXRoot{}, nFRoot{};
            (void)Bisect(std::numeric_limits<T>::digits,
                nP0, nP1, nXMid, nXMax, -1, nXRoot, nFRoot);

            T xRoot = static_cast<T>(nXRoot);
            T fRoot = static_cast<T>(nFRoot);
            return PolynomialRoot<T>(xRoot, fRoot, 1);
        }

        // The signFMin is +1 or -1. The signFMax is -signFMin, but because
        // signFMax is not used in the code explicitly, it does not need to
        // be passed to Bisect(...).
        template <typename T>
        static size_t Bisect(T p0, T p1, T xMin, T xMax, int32_t signFMin,
            T& xRoot, T& fAtXRoot)
        {
            // The bisection steps. The iteration algorithm terminates when
            // the midpoint of the current interval equals one of the interval
            // endpoints. At this time the interval endpoints are consecutive
            // floating-point numbers.
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);
            T x0 = xMin, x1 = xMax;
            size_t iteration{};
            for (iteration = 1; iteration < maxBisections; ++iteration)
            {
                xRoot = half * (x0 + x1);
                fAtXRoot = RobustSumOfProducts(xRoot, xRoot, p1, xRoot) + p0;

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive floating-point numbers in
                    // which case subdivision cannot produce a floating-point
                    // number between them.
                    break;
                }

                int32_t signFAtXRoot = (fAtXRoot > zero ? +1 : (fAtXRoot < zero ? -1 : 0));
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    break;
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
            return iteration;
        }

        static size_t Bisect(int32_t precision, Number const& p0, Number const& p1,
            Number const& xMin, Number const& xMax,
            int32_t signFMin, Number& xRoot, Number& fAtXRoot)
        {
            Number x0 = xMin, x1 = xMax;
            size_t iteration{};
            for (iteration = 1; iteration < maxBisections; ++iteration)
            {
                Number average = std::ldexp(x0 + x1, -1);  // (x0 + x1) / 2
                Convert(average, precision, FE_TONEAREST, xRoot);
                fAtXRoot = p0 + xRoot * (p1 + xRoot);

                if (xRoot == x0 || xRoot == x1)
                {
                    // x0 and x1 are consecutive rational numbers of the
                    // specified precision in which case subdivision cannot
                    // produce a rational number between them.
                    break;
                }

                int32_t signFAtXRoot = fAtXRoot.GetSign();
                if (signFAtXRoot == 0)
                {
                    // The function is exactly zero and a root is found.
                    break;
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
            return iteration;
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

        // Compute a*b+c*d robustly using floating-point arithmetic that
        // incorporates fused-multiply-add instructions. This is based on
        // https://pharr.org/matt/blog/2019/11/03/difference-of-floats
        // which references
        // https://people.eecs.berkeley.edu/~wkahan/Qdrtcs.pdf
        // that has the analysis of the algorithm.
        template <typename T>
        inline static T RobustSumOfProducts(T a, T b, T c, T d)
        {
            T stdCD = c * d;
            T error = std::fma(c, d, -stdCD);
            T fmaABCD = std::fma(a, b, stdCD);
            T result = fmaABCD + error;
            return result;
        }

#if defined(GTE_DEBUG_ROOTS_QUADRATIC)
        template <typename T>
        static void VerifySqrtInterval(Number const& nDiscr, SWInterval<T> const& iSqrtDiscr)
        {
            Number nLower(iSqrtDiscr[0]);
            Number nLowerSqr = nLower * nLower;
            LogAssert(
                nLowerSqr <= nDiscr,
                "Invalid interval minimum for sqrt(discr).");

            Number nUpper(iSqrtDiscr[1]);
            Number nUpperSqr = nUpper * nUpper;
            LogAssert(
                nDiscr <= nUpperSqr,
                "Invalid interval maximum for sqrt(discr).");
        }
#else
        template <typename T>
        static void VerifySqrtInterval(Number const&, SWInterval<T> const&)
        {
        }
#endif
    };
}
