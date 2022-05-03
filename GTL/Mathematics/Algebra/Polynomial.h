// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Implementation for polynomials of one or more variables. The number of
// variables must be specified at compile time. The definition is recursive.
//
// Polynomial<T, 1> represents
//   p(x[0]) = sum_{i=0}^{D} c[i] * x[0]^i
// where the c[i] are numbers of type T.
//
// Polynomial<T, 2> represents
//   p(x[0],x[1]) = sum_{i=0}^{D} c[i](x[0]) * x[1]^i,
// where the c[i](x[0]) are polynomials of type Polynomial<T, 1> in the
// variable x[0]. The degree(c[i]) can be any nonnegative number regardless
// of D = degree(p).
//
// Generally for N >= 2, let x = (x[0],x[1],...,x[N-1]) and let
// y = (x[0],x[1],...,x[N-2]) so that x = (y,x[N-1]). Polynomial<T, N-1>
// represents
//   p(x) = sum_{i=0}^{D} c[i](y) * x[N-1]^i,
// where the c[i](y) are polynomials of type Polynomial<T, N-2> in the
// variables y. The degree(c[i]) can be any nonnegative number regardless
// of D = degree(p).
//
// Polynomial<T, 0> is defined but has no members to terminate the
// template recursion.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/TypeTraits.h>
#include <algorithm>
#include <initializer_list>
#include <type_traits>
#include <utility>
#include <vector>

namespace gtl
{
    template <typename T, size_t N>
    class Polynomial
    {
    public:
        using value_type = T;

        // The type of the coefficients. For N = 1, the type is T. For N > 1,
        // the type is polynomial with one fewer variables.
        using CType = typename std::conditional<N == 1, T, Polynomial<T, N - 1>>::type;

        // Create a polynomial of degree 0 whose only coefficient is
        // initialized to zero.
        Polynomial()
            :
            mCoefficient(1, C_<CType>(0))
        {
            static_assert(
                N >= 1,
                "Invalid number of variables.");
        }

        // Create a polynomial of degree+1 whose coefficients are all
        // initialized to zero.
        Polynomial(size_t degree)
            :
            mCoefficient(degree + 1, C_<CType>(0))
        {
            static_assert(
                N >= 1,
                "Invalid number of variables.");
        }

        // Create a polynomial with specified coefficients. C++ 11 ensures
        // that the default constructor is called when 'coefficients' is an
        // empty list, so it is guaranteed inside the constructor that
        // 'coefficients' has at least one element.
        Polynomial(std::initializer_list<CType> coefficients)
            :
            mCoefficient(coefficients.size())
        {
            static_assert(
                N >= 1,
                "Invalid number of variables.");

            std::copy(coefficients.begin(), coefficients.end(), mCoefficient.begin());
            EliminateLeadingZeros(*this);
        }

        // Support for partial construction when the default constructor is
        // used and the desired degree is positive. The old coefficients
        // are preserved, although an input degree smaller than the current
        // degree will disallow access to coefficients with powers larger
        // than the input degree. When the input degree is larger than the
        // current degree, new coefficients are initialized to zero. NOTE:
        // If the degree of the polynomial increases and subsequent operations
        // on the polynomial do not assign the p[degree] term, the actual
        // degree of p is smaller than 'degree'. If this is possible in your
        // application, call EliminateLeadingZeros(p) before consuming the
        // polynomial further.
        inline void SetDegree(size_t degree)
        {
            size_t oldSize = mCoefficient.size();  // oldDegree = oldSize - 1
            size_t newSize = degree + 1;  // newDegree = degree
            mCoefficient.resize(newSize);
            if (newSize > oldSize)
            {
                CType const zero{};
                for (size_t i = oldSize; i < newSize; ++i)
                {
                    mCoefficient[i] = zero;
                }
            }
        }

        // Set the polynomial to a constant.
        Polynomial<T, N>& operator=(T const& constantTerm)
        {
            SetDegree(0);
            mCoefficient[0] = constantTerm;
            return *this;
        }

        // Member access.
        inline size_t GetDegree() const
        {
            // By design, mCoefficient.size() > 0, so no typecasting is
            // required for the subtraction.
            return mCoefficient.size() - 1;
        }

        inline CType const& operator[](size_t i) const
        {
            return mCoefficient[i];
        }

        inline CType& operator[](size_t i)
        {
            return mCoefficient[i];
        }

        inline std::vector<CType> const& GetCoefficients() const
        {
            return mCoefficient;
        }

        // Comparisons.
        inline bool operator==(Polynomial<T, N> const& p) const
        {
            return mCoefficient == p.mCoefficient;
        }

        inline bool operator!=(Polynomial<T, N> const& p) const
        {
            return mCoefficient != p.mCoefficient;
        }

        inline bool operator< (Polynomial<T, N> const& p) const
        {
            return mCoefficient < p.mCoefficient;
        }

        inline bool operator<=(Polynomial<T, N> const& p) const
        {
            return mCoefficient <= p.mCoefficient;
        }

        inline bool operator> (Polynomial<T, N> const& p) const
        {
            return mCoefficient > p.mCoefficient;
        }

        inline bool operator>=(Polynomial<T, N> const& p) const
        {
            return mCoefficient >= p.mCoefficient;
        }

        // Polynomial evaluation.
        template <size_t _N = N, TraitSelector<_N == 1> = 0>
        T operator()(T const& input) const
        {
            return operator()(&input);
        }

        template <size_t _N = N, TraitSelector<_N == 1> = 0>
        T operator()(T const* input) const
        {
            // By design, mCoefficient.size() > 0, so no typecasting is
            // required for the subtraction to compute jmax.
            size_t jmax = mCoefficient.size() - 1;
            T output = mCoefficient[jmax];
            if (jmax > 0)
            {
                // In this block, the initial i-value is guaranteed to be
                // nonnegative.
                for (size_t j = 0, i = jmax - 1; j < jmax; ++j, --i)
                {
                    output *= *input;
                    output += mCoefficient[i];
                }
            }
            return output;
        }

        template <size_t _N = N, TraitSelector<_N != 1> = 0>
        T operator()(T const* input) const
        {
            // By design, mCoefficient.size() > 0, so no typecasting is
            // required for the subtraction to compute jmax.
            size_t jmax = mCoefficient.size() - 1;
            T output = mCoefficient[jmax](input);
            if (jmax > 0)
            {
                // In this block, the initial i-value is guaranteed to be
                // nonnegative.
                T const& variable = input[N - 1];
                for (size_t j = 0, i = jmax - 1; j < jmax; ++j, --i)
                {
                    output *= variable;
                    output += mCoefficient[i](input);
                }
            }
            return output;
        }

    private:
        // The class is designed so that mCoefficient.size() >= 1.
        std::vector<CType> mCoefficient;
    };


    // Unary operations.
    template <typename T, size_t N>
    Polynomial<T, N> operator+(Polynomial<T, N> const& p)
    {
        return p;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator-(Polynomial<T, N> const& p)
    {
        Polynomial<T, N> result = p;
        for (size_t i = 0; i <= result.GetDegree(); ++i)
        {
            result[i] = -result[i];
        }
        return result;
    }

    // Linear algebraic operations.
    template <typename T, size_t N>
    Polynomial<T, N> operator+(Polynomial<T, N> const& p0, Polynomial<T, N> const& p1)
    {
        size_t const p0Degree = p0.GetDegree();
        size_t const p1Degree = p1.GetDegree();
        if (p0Degree >= p1Degree)
        {
            Polynomial<T, N> result = p0;
            for (size_t i = 0; i <= p1Degree; ++i)
            {
                result[i] += p1[i];
            }
            EliminateLeadingZeros(result);
            return result;
        }
        else
        {
            Polynomial<T, N> result = p1;
            for (size_t i = 0; i <= p0Degree; ++i)
            {
                result[i] += p0[i];
            }
            EliminateLeadingZeros(result);
            return result;
        }
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator+(Polynomial<T, N> const& p, T const& scalar)
    {
        Polynomial<T, N> result = p;
        result[0] += scalar;
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator+(T const& scalar, Polynomial<T, N> const& p)
    {
        Polynomial<T, N> result = p;
        result[0] += scalar;
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator+=(Polynomial<T, N>& p0, Polynomial<T, N> const& p1)
    {
        p0 = p0 + p1;
        return p0;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator+=(Polynomial<T, N>& p, T const& scalar)
    {
        p[0] += scalar;
        return p;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator-(Polynomial<T, N> const& p0, Polynomial<T, N> const& p1)
    {
        size_t const p0Degree = p0.GetDegree();
        size_t const p1Degree = p1.GetDegree();
        if (p0Degree >= p1Degree)
        {
            Polynomial<T, N> result = p0;
            for (size_t i = 0; i <= p1Degree; ++i)
            {
                result[i] -= p1[i];
            }
            EliminateLeadingZeros(result);
            return result;
        }
        else
        {
            Polynomial<T, N> result = -p1;
            for (size_t i = 0; i <= p0Degree; ++i)
            {
                result[i] += p0[i];
            }
            EliminateLeadingZeros(result);
            return result;
        }
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator-(Polynomial<T, N> const& p, T const& scalar)
    {
        Polynomial<T, N> result = p;
        result[0] -= scalar;
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator-(T const& scalar, Polynomial<T, N> const& p)
    {
        Polynomial<T, N> result = -p;
        result[0] += scalar;
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator-=(Polynomial<T, N>& p0, Polynomial<T, N> const& p1)
    {
        p0 = p0 - p1;
        return p0;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator-=(Polynomial<T, N>& p, T const& scalar)
    {
        p[0] -= scalar;
        return p;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator*(Polynomial<T, N> const& p, T const& scalar)
    {
        Polynomial<T, N> result = p;
        for (size_t i = 0; i <= result.GetDegree(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator*(T const& scalar, Polynomial<T, N> const& p)
    {
        Polynomial<T, N> result = p;
        for (size_t i = 0; i <= result.GetDegree(); ++i)
        {
            result[i] *= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator*=(Polynomial<T, N>& p, T const& scalar)
    {
        p = std::move(p * scalar);
        return p;
    }

    template <typename T, size_t N>
    Polynomial<T, N> operator/(Polynomial<T, N> const& p, T const& scalar)
    {
        Polynomial<T, N> result = p;
        for (size_t i = 0; i <= result.GetDegree(); ++i)
        {
            result[i] /= scalar;
        }
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator/=(Polynomial<T, N>& p, T const& scalar)
    {
        p = std::move(p / scalar);
        return p;
    }

    // Polynomial multiplication (group algebraic operation).
    template <typename T, size_t N>
    Polynomial<T, N> operator*(Polynomial<T, N> const& p0, Polynomial<T, N> const& p1)
    {
        size_t const p0Degree = p0.GetDegree();
        size_t const p1Degree = p1.GetDegree();
        Polynomial<T, N> result(p0Degree + p1Degree);  // initialized to zero
        for (size_t i0 = 0; i0 <= p0Degree; ++i0)
        {
            for (size_t i1 = 0; i1 <= p1Degree; ++i1)
            {
                result[i0 + i1] += p0[i0] * p1[i1];
            }
        }
        return result;
    }

    template <typename T, size_t N>
    Polynomial<T, N>& operator*=(Polynomial<T, N>& p0, Polynomial<T, N> const& p1)
    {
        p0 = p0 * p1;
        return p0;
    }


    // Operations for Polynomial<T, N>.

    // Test whether the polynomial is zero.
    template <typename T, size_t N>
    bool IsZero(Polynomial<T, N> const& p)
    {
        return p.GetDegree() == 0 && p[0] == typename Polynomial<T, N>::CType{};
    }

    // Test whether the polynomial is a constant.
    template <typename T, size_t N>
    bool IsConstant(Polynomial<T, N> const& p)
    {
        return p.GetDegree() == 0;
    }

    // Set all coefficients to the specified value.
    template <typename T, size_t N>
    void Fill(Polynomial<T, N>& p, T const& value)
    {
        for (size_t i = 0; i <= p.GetDegree(); ++i)
        {
            p[i] = value;
        }
    }

    // Eliminate any leading zeros in the polynomial except when the degree
    // is 0 and the coefficient is 0. The elimination is necessary when
    // arithmetic operations cause a decrease in the degree of the result.
    // For example, (1 + x + x^2) + (1 + 2*x - x^2) = (2 + 3*x). The inputs
    // both have degree 2, so the result is created with degree 2. The actual
    // degree is 1, so the array of coefficients must be resized. This
    // function is called internally by the arithmetic operators, but it is
    // exposed in the public interface for your applications.
    template <typename T, size_t N>
    void EliminateLeadingZeros(Polynomial<T, N>& p)
    {
        size_t size = p.GetDegree() + 1;
        if (size > 1)
        {
            auto zero = typename Polynomial<T, N>::CType{};
            size_t leading{};
            for (leading = size - 1; leading > 0; --leading)
            {
                if (p[leading] != zero)
                {
                    break;
                }
            }
            p.SetDegree(leading);
        }
    }


    // Operations specific to Polynomial<T, 1>.
    template <typename T>
    using Polynomial1 = Polynomial<T, 1>;

    // Scale the polynomial so the highest-degree term has coefficient 1.
    template <typename T>
    void MakeMonic(Polynomial1<T>& p)
    {
        EliminateLeadingZeros(p);
        size_t const degree = p.GetDegree();
        T& last = p[degree];
        if (last != C_<T>(1))
        {
            for (size_t i = 0; i < degree; ++i)
            {
                p[i] /= last;
            }
            last = C_<T>(1);
        }
    }

    template <typename T>
    Polynomial1<T> MultiplyByXToPower(size_t power, Polynomial1<T> const& p0)
    {
        if (power > 0)
        {
            size_t const p0Degree = p0.GetDegree();
            size_t const p1Degree = p0Degree + power;
            Polynomial1<T> p1(p1Degree);
            for (size_t i = 0, src = p0Degree, trg = p1Degree; i <= p0Degree; ++i, --src, --trg)
            {
                p1[trg] = p0[src];
            }
            for (size_t j = 0; j < power; ++j)
            {
                p1[j] = C_<T>(0);
            }
            return p1;
        }
        else
        {
            return p0;
        }
    }

    // Compute the derivative of the polynomial.
    template <typename T>
    Polynomial1<T> GetDerivative(Polynomial1<T> const& p)
    {
        size_t const degree = p.GetDegree();
        if (degree > 0)
        {
            Polynomial1<T> pder(degree - 1);
            for (size_t i0 = 0, i1 = 1; i0 < degree; ++i0, ++i1)
            {
                pder[i0] = p[i1] * static_cast<T>(i1);
            }
            return pder;
        }
        else
        {
            return Polynomial1<T>{};
        }
    }

    // Inversion (invpoly[i] = poly[degree-i] for 0 <= i <= degree).
    template <typename T>
    Polynomial1<T> GetInversion(Polynomial1<T> const& p)
    {
        size_t const degree = p.GetDegree();
        Polynomial1<T> inversion(degree);
        for (size_t i = 0; i <= degree; ++i)
        {
            inversion[i] = p[degree - i];
        }
        EliminateLeadingZeros(inversion);
        return inversion;
    }

    // Tranlation. For input polynomial p(x), the function returns p(x-x0).
    template <typename T>
    Polynomial1<T> GetTranslation(Polynomial1<T> const& p, T const& x0)
    {
        Polynomial1<T> linear{ -x0, C_<T>(1) };  // f(x) = x - x0
        size_t const degree = p.GetDegree();
        Polynomial1<T> translation{ p[degree] };
        for (size_t i = 1, j = degree - 1; i <= degree; ++i, --j)
        {
            translation = p[j] + linear * translation;
        }
        return translation;
    }


    //------------------------------------------------------------------------
    // The operations implemented next all rely on arbitrary-precision
    // arithmetic with division (BSRational) or without division (BSNumber).
    // The BSRational-based operations are slow for even moderate size
    // degrees because the number of bits grow exponentially, leading to
    // expensive multiplications of integers. The BSNumber-based operations
    // are faster, but they can also be slow when the degree is large. TODO:
    // It is essential to implement a UIntegerALU32 fast multiplication based
    // on the fast Fourier transform (polynomial multiplication is related to
    // convolution).
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // Operations requiring divisions.
    //------------------------------------------------------------------------

    // If the numerator polynomial is P(x) and the denominator is D(x) with
    // degree(P) >= degree(D), then P(x) = Q(x) * D(x) + R(x) where Q(x) is
    // the quotient with degree(Q) = degree(P) - degree(D) and R(x) is the
    // remainder with degree(R) < degree(D). If this routine is called with
    // degree(P) < degree(D), then Q = 0 and R = P are returned.
    template <typename Rational>
    void GetQR(
        Polynomial1<Rational> const& numerator,    // P(x)
        Polynomial1<Rational> const& denominator,  // D(x)
        Polynomial1<Rational>& quotient,           // Q(x)
        Polynomial1<Rational>& remainder)          // R(x)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            has_division_operator<Rational>::value,
            "The type must be arbitrary precision and have a division operator.");

        Rational const zero = C_<Rational>(0);
        GTL_ARGUMENT_ASSERT(
            denominator.GetDegree() > 0 || denominator[0] != zero,
            "The denominator must be a nonzero polynomial.");

        size_t const dDegree = denominator.GetDegree();
        Rational const& dLeading = denominator[dDegree];
        remainder = numerator;
        size_t rDegree = remainder.GetDegree();
        if (rDegree >= dDegree)
        {
            quotient.SetDegree(rDegree - dDegree);
            Fill(quotient, zero);
            while (rDegree >= dDegree && !IsZero(remainder))
            {
                size_t const rmdDegree = rDegree - dDegree;
                Rational const& rLeading = remainder[rDegree];
                Rational& qLeading = quotient[rmdDegree];
                qLeading = rLeading / dLeading;
                for (size_t i = rmdDegree, j = 0; i < rDegree; ++i, ++j)
                {
                    remainder[i] -= qLeading * denominator[j];
                }
                remainder[rDegree] = zero;
                EliminateLeadingZeros(remainder);
                rDegree = remainder.GetDegree();
            }
        }
        else
        {
            quotient = zero;
        }
    }

    // Compute the greatest common divisor (GCD) of two polynomials. The
    // leading coefficient of the GCD is not guaranteed to be 1.
    template <typename Rational>
    void GetGCD(
        Polynomial1<Rational> const& p0,
        Polynomial1<Rational> const& p1,
        Polynomial1<Rational>& gcd)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            has_division_operator<Rational>::value,
            "The type must be arbitrary precision and have a division operator.");

        if (!IsZero(p0))
        {
            if (!IsZero(p1))
            {
                Polynomial1<Rational> a{}, b{};
                if (p0.GetDegree() >= p1.GetDegree())
                {
                    a = p0;
                    b = p1;
                }
                else
                {
                    a = p1;
                    b = p0;
                }

                for (;;)
                {
                    Polynomial1<Rational> q{}, r{};
                    GetQR(a, b, q, r);
                    if (!IsZero(r))
                    {
                        a = std::move(b);
                        b = std::move(r);
                    }
                    else
                    {
                        break;
                    }
                }
                gcd = std::move(b);
            }
            else  // p1 == 0
            {
                gcd = p0;
            }
        }
        else  // p0 == 0
        {
            gcd = C_<Rational>(0);
        }

        if (gcd.GetDegree() == 0 && gcd[0] != C_<Rational>(0))
        {
            gcd[0] = C_<Rational>(1);
        }
    }

    // Compute the greatest common divisor (GCD) of two polynomials. The
    // leading coefficient of the GCD is not guaranteed to be 1. The returned
    // values q0 and q1 have the properties p0 = gcd * q0 and p1 = gcd * q1.
    // The algorithm is described at
    //   https://en.wikipedia.org/wiki/Polynomial_greatest_common_divisor
    // in Section 4.3.
    template <typename Rational>
    void GetExtendedGCD(
        Polynomial1<Rational> const& p0,
        Polynomial1<Rational> const& p1,
        Polynomial1<Rational>& gcd,
        Polynomial1<Rational>& q0,
        Polynomial1<Rational>& q1)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            has_division_operator<Rational>::value,
            "The type must be arbitrary precision and have a division operator.");

        Rational const zero = C_<Rational>(0);

        if (!IsZero(p0))
        {
            if (!IsZero(p1))
            {
                Polynomial1<Rational> r0{}, r1{}, r2{};
                if (p0.GetDegree() >= p1.GetDegree())
                {
                    r0 = p0;
                    r1 = p1;
                }
                else
                {
                    r0 = p1;
                    r1 = p0;
                }

                Polynomial1<Rational> r{};
                GetGCD(r0, r1, gcd);
                GetQR(r0, gcd, q0, r);
                GetQR(r1, gcd, q1, r);
            }
            else  // p1 = 0
            {
                gcd = p0;
                Rational const& gcdLeading = gcd[gcd.GetDegree()];
                q0 = gcdLeading;
                gcd /= gcdLeading;
                q1 = zero;
            }
        }
        else  // p0 = 0
        {
            gcd = zero;
            q0 = zero;
            q1 = zero;
        }
    }

    // Factor f = factor[0]*factor[1]^2*factor[2]^3*...*factor[n-1]^n
    // according to the algorithm described at
    //   https://en.wikipedia.org/wiki/Square-free_polynomial
    // for square-free factorization.
    template <typename Rational>
    void GetSquareFreeFactors(
        Polynomial1<Rational> const& p,
        std::vector<Polynomial1<Rational>>& factors)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            has_division_operator<Rational>::value,
            "The type must be arbitrary precision and have a division operator.");

        factors.clear();

        size_t const pDegree = p.GetDegree();
        if (pDegree <= 1)
        {
            // p(x) is a constant or linear polynomial, so it is already
            // square free.
            factors.push_back(p);
            return;
        }

        Polynomial1<Rational> p0 = p;
        Polynomial1<Rational> p1 = GetDerivative(p0);
        Polynomial1<Rational> g, q0, q1;
        GetExtendedGCD(p0, p1, g, q0, q1);
        if (g.GetDegree() == 0)
        {
            // The polynomials p(x) and p'(x) have greatest common divisor
            // that is a nonzero constant, so p(x) is already square free.
            factors.push_back(p);
            return;
        }

        // The power-0 factor is always 1.
        factors.push_back(Polynomial1<Rational>{ C_<Rational>(1) });

        // p0 = q0 * g
        // p1 = q1 * g
        // b1 = q0
        // c1 = q1
        // d1 = c1 - b1'

        do
        {
            p0 = std::move(q0);             // b1
            p1 = q1 - GetDerivative(p0);    // d1
            GetExtendedGCD(p0, p1, g, q0, q1);
            factors.push_back(g);
        }
        while (!IsZero(p1));
    }

    //------------------------------------------------------------------------
    // Operations not using divisions. The substring "Pseudo" indicates that
    // divisions are avoided.
    //------------------------------------------------------------------------

    // The Euclidean algorithm is as follows. If the numerator polynomial is
    // P(x) and the denominator is D(x) with degree(P) >= degree(D), then
    // P(x) = Q(x) * D(x) + R(x) where Q(x) is the quotient with degree(Q) =
    // degree(P) - degree(D) and R(x) is the remainder with degree(R) <
    // degree(D). If this routine is called with degree(P) < degree(D), then
    // Q = 0 and R = P are returned.
    //
    // Using BSRational, GetQR(P,D,Q,R) computes Q and R from P and D, but the
    // algorithm uses division of coefficients.
    //
    // Using BSNumber, no divisions are allowed. GetPseudoQR(P,D,Q0,R0)
    // computes A, Q0 and R0. If L = Q0[degree(Q0)] and the Euclidean
    // algorithm requires k iterations to obtain R with degree(R) < degree(D),
    // then A = L^k and A * P(x) = Q0(x) * D(x) + R0(x). Observe that
    // Q(x) = Q0(x) / A and R(x) = R0(x) / A.
    template <typename Rational>
    void GetPseudoQR(
        Polynomial1<Rational> const& numerator,
        Polynomial1<Rational> const& denominator,
        Rational& amplitude,
        Polynomial1<Rational>& quotient,
        Polynomial1<Rational>& remainder)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            !has_division_operator<Rational>::value,
            "The type must be arbitrary precision without a division operator.");

        GTL_ARGUMENT_ASSERT(
            !IsZero(denominator),
            "The divisor must be a nonzero polynomial.");

        Rational const zero = C_<Rational>(0);
        size_t dDegree = denominator.GetDegree();
        Rational const& dLeading = denominator[dDegree];
        remainder = numerator;
        size_t rDegree = remainder.GetDegree();
        if (rDegree >= dDegree)
        {
            quotient.SetDegree(rDegree - dDegree);
            Fill(quotient, zero);
            while (rDegree >= dDegree && !IsZero(remainder))
            {
                size_t const rmdDegree = rDegree - dDegree;
                Rational const& rLeading = remainder[rDegree];
                quotient[rmdDegree] = rLeading;

                for (size_t i = 0; i < rDegree; ++i)
                {
                    remainder[i] = dLeading * remainder[i];
                }
                for (size_t i = rmdDegree, j = 0; i < rDegree; ++i, ++j)
                {
                    remainder[i] -= rLeading * denominator[j];
                }
                remainder[rDegree] = zero;
                EliminateLeadingZeros(remainder);
                rDegree = remainder.GetDegree();
            }

            amplitude = C_<Rational>(1);
            for (size_t i = 0; i <= quotient.GetDegree(); ++i)
            {
                if (quotient[i] != zero)
                {
                    quotient[i] *= amplitude;
                    amplitude *= dLeading;
                }
            }
        }
        else
        {
            quotient = zero;
        }
    }

    // Compute the greatest common divisor (GCD) of two polynomials. The
    // leading coefficient of the GCD is not guaranteed to be 1 except
    // when the GCD is a constant.
    template <typename Rational>
    void GetPseudoGCD(
        Polynomial1<Rational> const& p0,
        Polynomial1<Rational> const& p1,
        Polynomial1<Rational>& gcd)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            !has_division_operator<Rational>::value,
            "The type must be arbitrary precision without a division operator.");

        if (!IsZero(p0))
        {
            if (!IsZero(p1))
            {
                Polynomial1<Rational> a{}, b{};
                if (p0.GetDegree() >= p1.GetDegree())
                {
                    a = p0;
                    b = p1;
                }
                else
                {
                    a = p1;
                    b = p0;
                }

                for (;;)
                {
                    Rational amplitude{};
                    Polynomial1<Rational> q{}, r{};
                    GetPseudoQR(a, b, amplitude, q, r);
                    if (!IsZero(r))
                    {
                        a = std::move(b);
                        b = std::move(r);
                    }
                    else
                    {
                        break;
                    }
                }
                gcd = std::move(b);
            }
            else  // p1 == 0
            {
                gcd = p0;
            }
        }
        else  // p0 == 0
        {
            gcd = C_<Rational>(0);
        }

        if (gcd.GetDegree() == 0 && gcd[0] != C_<Rational>(0))
        {
            gcd[0] = C_<Rational>(1);
        }
    }

    // Compute the greatest common divisor (GCD) of two polynomials. The
    // leading coefficient of the GCD is not guaranteed to be 1. The returned
    // values q0 and q1 have the properties a0 * p0 = gcd * q0 and
    // a1 * p1 = gcd * q1.
    template <typename Rational>
    void GetPseudoExtendedGCD(
        Polynomial1<Rational> const& p0,
        Polynomial1<Rational> const& p1,
        Polynomial1<Rational>& gcd,
        Rational& a0,
        Rational& a1,
        Polynomial1<Rational>& q0,
        Polynomial1<Rational>& q1)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            !has_division_operator<Rational>::value,
            "The type must be arbitrary precision without a division operator.");

        Rational const zero = C_<Rational>(0);

        if (!IsZero(p0))
        {
            if (!IsZero(p1))
            {
                Polynomial1<Rational> r0{}, r1{}, r2{};
                if (p0.GetDegree() >= p1.GetDegree())
                {
                    r0 = p0;
                    r1 = p1;
                }
                else
                {
                    r0 = p1;
                    r1 = p0;
                }

                Polynomial1<Rational> r{};
                GetPseudoGCD(r0, r1, gcd);
                GetPseudoQR(r0, gcd, a0, q0, r);
                GetPseudoQR(r1, gcd, a1, q1, r);
            }
            else  // p1 = 0
            {
                gcd = p0;
                q0 = C_<Rational>(1);
                q1 = zero;
            }
        }
        else  // p0 = 0
        {
            gcd = zero;
            q0 = zero;
            q1 = zero;
        }
    }

    // Factor f = factor[0]*factor[1]^2*factor[2]^3*...*factor[n-1]^n
    // according to the algorithm described at
    //   https://en.wikipedia.org/wiki/Square-free_polynomial
    // for square-free factorization. Modifications were made to avoid
    // divisions.
    template <typename Rational>
    void GetPseudoSquareFreeFactors(
        Polynomial1<Rational> const& p,
        std::vector<Polynomial1<Rational>>& factors)
    {
        static_assert(
            is_arbitrary_precision<Rational>::value &&
            !has_division_operator<Rational>::value,
            "The type must be arbitrary precision without a division operator.");

        factors.clear();

        size_t const pDegree = p.GetDegree();
        if (pDegree <= 1)
        {
            // p(x) is a constant or linear polynomial, so it is already
            // square free.
            factors.push_back(p);
            return;
        }

        Polynomial1<Rational> p0 = p;
        Polynomial1<Rational> p1 = GetDerivative(p0);
        Polynomial1<Rational> g{}, q0{}, q1{};
        Rational m0{}, m1{};
        GetPseudoExtendedGCD(p0, p1, g, m0, m1, q0, q1);
        if (g.GetDegree() == 0)
        {
            // The polynomials p(x) and p'(x) have greatest common divisor
            // that is a nonzero constant, so p(x) is already square free.
            factors.push_back(p);
            return;
        }

        // The power-0 factor is always 1.
        factors.push_back(Polynomial1<Rational>{ C_<Rational>(1) });

        // p0 = (q0 / m0) * g
        // p1 = (q1 / m1) * g
        // b1 = q0 / m0 = m1 * q0 / (m0 * m1)
        // c1 = q1 / m1 = m0 * q1 / (m0 * m1)
        // d1 = c1 - b1' = (m0 * q1 - m1 * q0') / (m0 * m1)

        do
        {
            p0 = m1 * q0;                       // b1
            p1 = m0 * q1 - GetDerivative(p0);   // d1
            GetPseudoExtendedGCD(p0, p1, g, m0, m1, q0, q1);
            factors.push_back(g);
        }
        while (!IsZero(p1));
    }
}
