// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

#include <GTL/Mathematics/Arithmetic/ArbitraryPrecision.h>
#include <algorithm>
#include <array>
#include <type_traits>

// The interval [e0,e1] must satisfy e0 <= e1. If you want exceptions thrown
// when e0 > e1, add the preprocessor symbol GTL_THROW_ON_INVALID_APINTERVAL
// to the global defines passed to the compiler.

namespace gtl
{
    // The APType must be an arbitrary-precision type.
    template <typename APType>
    class APInterval
    {
    public:
        // Once an interval is created, it is immutable. The constructor
        // APInterval(APType) creates the degenerate interval [e,e].
        APInterval()
            :
            mEndpoints{ C_<APType>(0), C_<APType>(0) }
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");
        }

        APInterval(APInterval const& other)
            :
            mEndpoints(other.mEndpoints)
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");
        }

        APInterval(APType const& e)
            :
            mEndpoints{ e, e }
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");
        }

        APInterval(APType const& e0, APType const& e1)
            :
            mEndpoints{ e0, e1 }
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");

#if defined(GTL_THROW_ON_INVALID_APINTERVAL)
            GTL_ARGUMENT_ASSERT(
                mEndpoints[0] <= mEndpoints[1],
                "Incorrect order of endpoints.");
#endif
        }

        APInterval(std::array<APType, 2> const& endpoint)
            :
            mEndpoints(endpoint)
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");

#if defined(GTL_THROW_ON_INVALID_APINTERVAL)
            GTL_ARGUMENT_ASSERT(
                mEndpoints[0] <= mEndpoints[1],
                "Incorrect order of endpoints.");
#endif
        }

        APInterval& operator=(APInterval const& other)
        {
            static_assert(
                is_arbitrary_precision<APType>::value,
                "Invalid type.");

            mEndpoints = other.mEndpoints;
            return *this;
        }

        // Member access. It is only possible to read the endpoints. You
        // cannot modify the endpoints outside the arithmetic operations.
        inline APType operator[](size_t i) const
        {
            return mEndpoints[i];
        }

        inline std::array<APType, 2> GetEndpoints() const
        {
            return mEndpoints;
        }

        // Arithmetic operations to compute intervals at the leaf nodes of an
        // expression tree. Such nodes correspond to the raw floating-point
        // variables of the expression. The non-class operators defined after
        // the class definition are used to compute intervals at the interior
        // nodes of the expression tree.
        inline static APInterval Add(APType const& u, APType const& v)
        {
            return APInterval(u + v);
        }

        inline static APInterval Sub(APType const& u, APType const& v)
        {
            return APInterval(u - v);
        }

        inline static APInterval Mul(APType const& u, APType const& v)
        {
            return APInterval(u * v);
        }

        template <typename _APType = APType, IsDivisionType<_APType> = 0>
        inline static APInterval Div(APType const& u, APType const& v)
        {
            if (v != C_<APType>(0))
            {
                return APInterval(u / v);
            }
            else
            {
                // Division by zero does not lead to a determinate interval.
                // Just return the entire set of real numbers.
                return Reals();
            }
        }

        // The remainder of the functions defined here are for internal use.
        // These are used by the non-class operators defined after the class
        // definition.
        inline static APInterval Add(APType const& u0, APType const& u1,
            APType const& v0, APType const& v1)
        {
            return APInterval(u0 + v0, u1 + v1);
        }

        inline static APInterval Sub(APType const& u0, APType const& u1,
            APType const& v0, APType const& v1)
        {
            return APInterval(u0 - v1, u1 - v0);
        }

        inline static APInterval Mul(APType const& u0, APType const& u1,
            APType const& v0, APType const& v1)
        {
            return APInterval(u0 * v0, u1 * v1);
        }

        inline static APInterval Mul2(APType const& u0, APType const& u1,
            APType const& v0, APType const& v1)
        {
            return APInterval(std::min(u0 * v1, u1 * v0), std::max(u0 * v0, u1 * v1));
        }

        template <typename _APType = APType, IsDivisionType<_APType> = 0>
        inline static APInterval Div(APType const& u0, APType const& u1, APType const& v0, APType const& v1)
        {
            return APInterval(u0 / v1, u1 / v0);
        }

        template <typename _APType = APType, IsDivisionType<_APType> = 0>
        inline static APInterval Reciprocal(APType const& v0, APType const& v1)
        {
            return APInterval(C_<APType>(1) / v1, C_<APType>(1) / v0);
        }

        template <typename _APType = APType, IsDivisionType<_APType> = 0>
        inline static APInterval ReciprocalDown(APType const& v)
        {
            // TODO: For now, a sign of +2 is a signal that posinf is an
            // arbitrary-precision number that represents +infinity.
            APType posinf(0);
            posinf.SetSign(+2);
            return APInterval(C_<APType>(1) / v, posinf);
        }

        template <typename _APType = APType, IsDivisionType<_APType> = 0>
        inline static APInterval ReciprocalUp(APType const& v)
        {
            // TODO: For now, a sign of -2 is a signal that neginf is an
            // arbitrary-precision number that represents -infinity.
            APType neginf(0);
            neginf.SetSign(-2);
            return APInterval<APType>(neginf, C_<APType>(1) / v);
        }

        inline static APInterval Reals()
        {
            // TODO: For now, a sign of +2 is a signal that posinf is an
            // arbitrary-precision number that represents +infinity and a
            // sign of -2 is a signal that neginf is an arbitrary-precision
            // number that represents -infinity.
            APType posinf(0), neginf(0);
            posinf.SetSign(+2);
            neginf.SetSign(-2);
            return APInterval(neginf, posinf);
        }

    private:
        std::array<APType, 2> mEndpoints;

        friend class UnitTestAPInterval;
    };

    // Unary operations. Negation of [e0,e1] produces [-e1,-e0]. This
    // operation needs to be supported in the sense of negating a
    // "number" in an arithmetic expression.
    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u)
    {
        return u;
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u)
    {
        return APInterval<APType>(-u[1], -u[0]);
    }

    // Addition operations.
    template <typename APType>
    APInterval<APType> operator+(APType const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Add(u, u, v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u, APType const& v)
    {
        return APInterval<APType>::Add(u[0], u[1], v, v);
    }

    template <typename APType>
    APInterval<APType> operator+(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Add(u[0], u[1], v[0], v[1]);
    }

    // Subtraction operations.
    template <typename APType>
    APInterval<APType> operator-(APType const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Sub(u, u, v[0], v[1]);
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u, APType const& v)
    {
        return APInterval<APType>::Sub(u[0], u[1], v, v);
    }

    template <typename APType>
    APInterval<APType> operator-(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        return APInterval<APType>::Sub(u[0], u[1], v[0], v[1]);
    }

    // Multiplication operations.
    template <typename APType>
    APInterval<APType> operator*(APType const& u, APInterval<APType> const& v)
    {
        if (u >= C_<APType>(0))
        {
            return APInterval<APType>::Mul(u, u, v[0], v[1]);
        }
        else
        {
            return APInterval<APType>::Mul(u, u, v[1], v[0]);
        }
    }

    template <typename APType>
    APInterval<APType> operator*(APInterval<APType> const& u, APType const& v)
    {
        if (v >= C_<APType>(0))
        {
            return APInterval<APType>::Mul(u[0], u[1], v, v);
        }
        else
        {
            return APInterval<APType>::Mul(u[1], u[0], v, v);
        }
    }

    template <typename APType>
    APInterval<APType> operator*(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        if (u[0] >= C_<APType>(0))
        {
            if (v[0] >= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[0], u[1], v[0], v[1]);
            }
            else if (v[1] <= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[1], u[0], v[0], v[1]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul(u[1], u[1], v[0], v[1]);
            }
        }
        else if (u[1] <= C_<APType>(0))
        {
            if (v[0] >= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[0], u[1], v[1], v[0]);
            }
            else if (v[1] <= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[1], u[0], v[1], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul(u[0], u[0], v[1], v[0]);
            }
        }
        else // u[0] < 0 < u[1]
        {
            if (v[0] >= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[0], u[1], v[1], v[1]);
            }
            else if (v[1] <= C_<APType>(0))
            {
                return APInterval<APType>::Mul(u[1], u[0], v[0], v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Mul2(u[0], u[1], v[0], v[1]);
            }
        }
    }

    // Division operations. If the divisor interval is [v0,v1] with
    // v0 < 0 < v1, then the returned interval is (-infinity,+infinity)
    // instead of Union((-infinity,1/v0),(1/v1,+infinity)). An application
    // should try to avoid this case by branching based on [v0,0] and [0,v1].
    template <typename APType>
    APInterval<APType> operator/(APType const& u, APInterval<APType> const& v)
    {
        if (v[0] > C_<APType>(0) || v[1] < C_<APType>(0))
        {
            return u * APInterval<APType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == C_<APType>(0))
            {
                return u * APInterval<APType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == C_<APType>(0))
            {
                return u * APInterval<APType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Reals();
            }
        }
    }

    template <typename APType>
    APInterval<APType> operator/(APInterval<APType> const& u, APType const& v)
    {
        if (v > C_<APType>(0))
        {
            return APInterval<APType>::Div(u[0], u[1], v, v);
        }
        else if (v < C_<APType>(0))
        {
            return APInterval<APType>::Div(u[1], u[0], v, v);
        }
        else // v = 0
        {
            return APInterval<APType>::Reals();
        }
    }

    template <typename APType>
    APInterval<APType> operator/(APInterval<APType> const& u, APInterval<APType> const& v)
    {
        if (v[0] > C_<APType>(0) || v[1] < C_<APType>(0))
        {
            return u * APInterval<APType>::Reciprocal(v[0], v[1]);
        }
        else
        {
            if (v[0] == C_<APType>(0))
            {
                return u * APInterval<APType>::ReciprocalDown(v[1]);
            }
            else if (v[1] == C_<APType>(0))
            {
                return u * APInterval<APType>::ReciprocalUp(v[0]);
            }
            else // v[0] < 0 < v[1]
            {
                return APInterval<APType>::Reals();
            }
        }
    }
}
