// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// SWInterval is an implementation of interval arithmetic in software. It is
// preferred that interval arithmetic use the floating-point hardware, but
// g++ appears not to support changes to the floating-point environment via
// the support in <cfenv>. The constructors that take two distinct numeric
// inputs create an interval [e0,e1] with e0 <= e1. If you want exceptions
// thrown when e0 > e1, add the preprocessor symbol
// GT_THROW_ON_INVALID_SWINTERVAL to the global defines passed to the
// compiler.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>

namespace gtl
{
    // The T must be 'float' or 'double'.
    template <typename T>
    class SWInterval
    {
    public:
        // Construction. This is the only way to create an interval. All such
        // intervals are immutable once created. The constructor SWInterval(T)
        // is used to create the degenerate interval [e,e].
        SWInterval()
            :
            mEndpoints{ C_<T>(0), C_<T>(0) }
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");
        }

        SWInterval(SWInterval const& other)
            :
            mEndpoints(other.mEndpoints)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");
        }

        SWInterval(T const& e)
            :
            mEndpoints{ e, e }
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");
        }

        SWInterval(T const& e0, T const& e1)
            :
            mEndpoints{ e0, e1 }
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

#if defined(GT_THROW_ON_INVALID_SWINTERVAL)
            GTL_ARGUMENT_ASSERT(
                mEndpoints[0] <= mEndpoints[1],
                "Incorrect order of endpoints.");
#endif
        }

        SWInterval(std::array<T, 2> const& endpoint)
            :
            mEndpoints(endpoint)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

#if defined(GT_THROW_ON_INVALID_SWINTERVAL)
            GTL_ARGUMENT_ASSERT(
                mEndpoints[0] <= mEndpoints[1],
                "Incorrect order of endpoints.");
#endif
        }

        SWInterval(int32_t e)
            :
            mEndpoints{ static_cast<T>(e), static_cast<T>(e) }
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");
        }

        SWInterval(int32_t e0, int32_t e1)
            :
            mEndpoints{ static_cast<T>(e0), static_cast<T>(e1) }
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

#if defined(GT_THROW_ON_INVALID_SWINTERVAL)
            GTL_ARGUMENT_ASSERT(
                mEndpoints[0] <= mEndpoints[1],
                "Incorrect order of endpoints.");
#endif
        }

        SWInterval& operator=(SWInterval const& other)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Member access. It is only possible to read the endpoints. You
        // cannot modify the endpoints outside the arithmetic operations.
        inline T const& operator[](size_t i) const
        {
            return mEndpoints[i];
        }

        inline std::array<T, 2> const& GetEndpoints() const
        {
            return mEndpoints;
        }

        // Arithmetic operations to compute intervals at the leaf nodes of
        // an expression tree. Such nodes correspond to the raw floating-point
        // variables of the expression. The non-class operators defined after
        // the class definition are used to compute intervals at the interior
        // nodes of the expression tree.
        inline static SWInterval Add(T const& u, T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            T add = u + v;
            w.mEndpoints[0] = std::nextafter(add, -infinity);
            w.mEndpoints[1] = std::nextafter(add, +infinity);
            return w;
        }

        inline static SWInterval Sub(T const& u, T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            T sub = u - v;
            w.mEndpoints[0] = std::nextafter(sub, -infinity);
            w.mEndpoints[1] = std::nextafter(sub, +infinity);
            return w;
        }

        inline static SWInterval Mul(T const& u, T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            T mul = u * v;
            w.mEndpoints[0] = std::nextafter(mul, -infinity);
            w.mEndpoints[1] = std::nextafter(mul, +infinity);
            return w;
        }

        inline static SWInterval Div(T const& u, T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            if (v != C_<T>(0))
            {
                SWInterval w{};
                T div = u / v;
                w.mEndpoints[0] = std::nextafter(div, -infinity);
                w.mEndpoints[1] = std::nextafter(div, +infinity);
                return w;
            }
            else
            {
                // Division by zero does not lead to a determinate SWInterval.
                // Return the entire set of real numbers.
                return SWInterval(-infinity, +infinity);
            }
        }

        // Unary operations. Negation of [e0,e1] produces [-e1,-e0]. This
        // operation needs to be supported in the sense of negating a
        // "number" in an arithmetic expression.
        inline friend SWInterval operator+(SWInterval const& u)
        {
            return u;
        }

        inline friend SWInterval operator-(SWInterval const& u)
        {
            return SWInterval(-u.mEndpoints[1], -u.mEndpoints[0]);
        }

        // Addition operations.
        inline friend SWInterval operator+(T const& u, SWInterval const& v)
        {
            return Add(u, u, v[0], v[1]);
        }

        inline friend SWInterval operator+(SWInterval<T> const& u, T const& v)
        {
            return Add(u[0], u[1], v, v);
        }

        inline friend SWInterval operator+(SWInterval const& u, SWInterval const& v)
        {
            return Add(u[0], u[1], v[0], v[1]);
        }

        inline friend SWInterval& operator+=(SWInterval& u, T const& v)
        {
            u = u + v;
            return u;
        }

        inline friend SWInterval& operator+=(SWInterval& u, SWInterval const& v)
        {
            u = u + v;
            return u;
        }

        // Subtraction operations.
        inline friend SWInterval operator-(T const& u, SWInterval const& v)
        {
            return Sub(u, u, v[0], v[1]);
        }

        inline friend SWInterval operator-(SWInterval const& u, T const& v)
        {
            return Sub(u[0], u[1], v, v);
        }

        inline friend SWInterval operator-(SWInterval const& u, SWInterval const& v)
        {
            return Sub(u[0], u[1], v[0], v[1]);
        }

        inline friend SWInterval& operator-=(SWInterval& u, T const& v)
        {
            u = u - v;
            return u;
        }

        inline friend SWInterval& operator-=(SWInterval& u, SWInterval const& v)
        {
            u = u - v;
            return u;
        }

        // Multiplication operations.
        inline friend SWInterval operator*(T const& u, SWInterval const& v)
        {
            if (u >= C_<T>(0))
            {
                return Mul(u, u, v[0], v[1]);
            }
            else
            {
                return Mul(u, u, v[1], v[0]);
            }
        }

        inline friend SWInterval operator*(SWInterval<T> const& u, T const& v)
        {
            if (v >= C_<T>(0))
            {
                return Mul(u[0], u[1], v, v);
            }
            else
            {
                return Mul(u[1], u[0], v, v);
            }
        }

        inline friend SWInterval operator*(SWInterval const& u, SWInterval const& v)
        {
            if (u[0] >= C_<T>(0))
            {
                if (v[0] >= C_<T>(0))
                {
                    return Mul(u[0], u[1], v[0], v[1]);
                }
                else if (v[1] <= C_<T>(0))
                {
                    return Mul(u[1], u[0], v[0], v[1]);
                }
                else // v[0] < 0 < v[1]
                {
                    return Mul(u[1], u[1], v[0], v[1]);
                }
            }
            else if (u[1] <= C_<T>(0))
            {
                if (v[0] >= C_<T>(0))
                {
                    return Mul(u[0], u[1], v[1], v[0]);
                }
                else if (v[1] <= C_<T>(0))
                {
                    return Mul(u[1], u[0], v[1], v[0]);
                }
                else // v[0] < 0 < v[1]
                {
                    return Mul(u[0], u[0], v[1], v[0]);
                }
            }
            else // u[0] < 0 < u[1]
            {
                if (v[0] >= C_<T>(0))
                {
                    return Mul(u[0], u[1], v[1], v[1]);
                }
                else if (v[1] <= C_<T>(0))
                {
                    return Mul(u[1], u[0], v[0], v[0]);
                }
                else // v[0] < 0 < v[1]
                {
                    return Mul2(u[0], u[1], v[0], v[1]);
                }
            }
        }

        inline friend SWInterval& operator*=(SWInterval& u, T const& v)
        {
            u = u * v;
            return u;
        }

        inline friend SWInterval& operator*=(SWInterval& u, SWInterval const& v)
        {
            u = u * v;
            return u;
        }

        // Division operations. If the divisor SWInterval is [v0,v1] with
        // v0 < 0 < v1, then the returned SWInterval is (-inf,+inf) instead of
        // Union((-inf,1/v0),(1/v1,+inf)). An application should try to avoid
        // this case by branching based on [v0,0] and [0,v1].
        inline friend SWInterval operator/(T const& u, SWInterval<T> const& v)
        {
            if (v[0] > C_<T>(0) || v[1] < C_<T>(0))
            {
                return u * Reciprocal(v[0], v[1]);
            }
            else
            {
                if (v[0] == C_<T>(0))
                {
                    return u * ReciprocalDown(v[1]);
                }
                else if (v[1] == C_<T>(0))
                {
                    return u * ReciprocalUp(v[0]);
                }
                else // v[0] < 0 < v[1]
                {
                    T constexpr infinity = std::numeric_limits<T>::infinity();
                    return SWInterval(-infinity, +infinity);
                }
            }
        }

        inline friend SWInterval<T> operator/(SWInterval<T> const& u, T const& v)
        {
            if (v > C_<T>(0))
            {
                return Div(u[0], u[1], v, v);
            }
            else if (v < C_<T>(0))
            {
                return Div(u[1], u[0], v, v);
            }
            else // v = 0
            {
                T constexpr infinity = std::numeric_limits<T>::infinity();
                return SWInterval(-infinity, +infinity);
            }
        }

        inline friend SWInterval operator/(SWInterval const& u, SWInterval const& v)
        {
            if (v[0] > C_<T>(0) || v[1] < C_<T>(0))
            {
                return u * Reciprocal(v[0], v[1]);
            }
            else
            {
                if (v[0] == C_<T>(0))
                {
                    return u * ReciprocalDown(v[1]);
                }
                else if (v[1] == C_<T>(0))
                {
                    return u * ReciprocalUp(v[0]);
                }
                else // v[0] < 0 < v[1]
                {
                    T constexpr infinity = std::numeric_limits<T>::infinity();
                    return SWInterval(-infinity, +infinity);
                }
            }
        }

        inline friend SWInterval& operator/=(SWInterval& u, T const& v)
        {
            u = u / v;
            return u;
        }

        inline friend SWInterval& operator/=(SWInterval& u, SWInterval const& v)
        {
            u = u / v;
            return u;
        }

    private:
        // These are used by the friend operators defined previously.
        inline static SWInterval Add(T const& u0, T const& u1, T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            w.mEndpoints[0] = std::nextafter(u0 + v0, -infinity);
            w.mEndpoints[1] = std::nextafter(u1 + v1, +infinity);
            return w;
        }

        inline static SWInterval Sub(T const& u0, T const& u1, T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            w.mEndpoints[0] = std::nextafter(u0 - v1, -infinity);
            w.mEndpoints[1] = std::nextafter(u1 - v0, +infinity);
            return w;
        }

        inline static SWInterval Mul(T const& u0, T const& u1, T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            w.mEndpoints[0] = std::nextafter(u0 * v0, -infinity);
            w.mEndpoints[1] = std::nextafter(u1 * v1, +infinity);
            return w;
        }

        inline static SWInterval Mul2(T const& u0, T const& u1, T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            T u0mv1 = std::nextafter(u0 * v1, -infinity);
            T u1mv0 = std::nextafter(u1 * v0, -infinity);
            T u0mv0 = std::nextafter(u0 * v0, +infinity);
            T u1mv1 = std::nextafter(u1 * v1, +infinity);
            return SWInterval<T>(std::min(u0mv1, u1mv0), std::max(u0mv0, u1mv1));
        }

        inline static SWInterval Div(T const& u0, T const& u1, T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            w.mEndpoints[0] = std::nextafter(u0 / v1, -infinity);
            w.mEndpoints[1] = std::nextafter(u1 / v0, +infinity);
            return w;
        }

        inline static SWInterval Reciprocal(T const& v0, T const& v1)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            SWInterval w{};
            w.mEndpoints[0] = std::nextafter(C_<T>(1) / v1, -infinity);
            w.mEndpoints[1] = std::nextafter(C_<T>(1) / v0, +infinity);
            return w;
        }

        inline static SWInterval ReciprocalDown(T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            T recpv = std::nextafter(C_<T>(1) / v, -infinity);
            return SWInterval<T>(recpv, +infinity);
        }

        inline static SWInterval ReciprocalUp(T const& v)
        {
            static_assert(
                std::is_floating_point<T>::value,
                "Invalid type.");

            T constexpr infinity = std::numeric_limits<T>::infinity();

            T recpv = std::nextafter(C_<T>(1) / v, +infinity);
            return SWInterval<T>(-infinity, recpv);
        }

        std::array<T, 2> mEndpoints;

        friend class UnitTestSWInterval;
    };
}
