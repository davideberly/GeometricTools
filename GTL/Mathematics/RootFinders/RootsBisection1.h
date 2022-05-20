// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// Estimate a root on an interval [tMin,tMax] for a continuous function F(t)
// defined on that interval. If a root is found, the function returns it via
// tRoot. Additionally, fAtTRoot = F(tRoot) is returned in case the caller
// wants to know how close to zero the function is at the root; numerical
// rounding errors can cause fAtTRoot not to be exactly zero. The returned
// uint32_t is the number of iterations used by the bisector. If that number
// is 0, F(tMin)*F(tMax) > 0 and it is unknown whether [tMin,tMax] contains
// a root. If that number is 1, either F(tMin) = 0 or F(tMax) = 0 (exactly),
// and tRoot is the corresponding interval endpoint. If that number is 2 or
// larger, the bisection is applied until tRoot is found for which F(tRoot)
// is exactly 0 or until the current root estimate is equal to tMin or tMax.
// The latter conditions can occur because of the fixed precision used in
// the computations: 24-bit precision for 'float', 53-bit precision for
// 'double' or a user-specified precision for arbitrary-precision numbers.

#include <GTL/Mathematics/Arithmetic/ArbitraryPrecision.h>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>

namespace gtl
{
    template <typename T>
    class RootsBisection1
    {
    public:
        // Use this constructor when T is a floating-point type.
        template <typename Numeric = T, IsFPType<Numeric> = 0>
        RootsBisection1(size_t maxIterations, size_t = 0)
            :
            mMaxIterations(maxIterations),
            mPrecision(0),
            mNumIterations(0),
            mFinalTMin(C_<T>(0)),
            mFinalTMax(C_<T>(0)),
            mFinalFMin(C_<T>(0)),
            mFinalFMax(C_<T>(0))
        {
            GTL_ARGUMENT_ASSERT(
                mMaxIterations > 0,
                "The maximum iterations must be positive.");
        }

        // Use this constructor when T is an arbitrary-precision type. If you
        // want infinite precision (no rounding of any computational results),
        // set precision to std::numeric_limits<size_t>::max(). For rounding
        // of each computational result throughout the process, set precision
        // to be a number smaller than std::numeric_limits<size_t>::max().
        template <typename Numeric = T, IsAPType<Numeric> = 0>
        RootsBisection1(size_t maxIterations, size_t precision)
            :
            mMaxIterations(maxIterations),
            mPrecision(precision),
            mNumIterations(0),
            mFinalTMin(C_<T>(0)),
            mFinalTMax(C_<T>(0)),
            mFinalFMin(C_<T>(0)),
            mFinalFMax(C_<T>(0))
        {
            GTL_ARGUMENT_ASSERT(
                mPrecision > 0,
                "The precision must be positive.");

            GTL_ARGUMENT_ASSERT(
                mMaxIterations > 0,
                "The maximum iterations must be positive.");
        }

        // Use this function when F(tMin) and F(tMax) are not already known.
        bool operator()(std::function<T(T const&)> const& F,
            T const& tMin, T const& tMax, T& tRoot, T& fAtTRoot)
        {
            GTL_ARGUMENT_ASSERT(
                tMin < tMax,
                "Invalid ordering of t-interval endpoints.");

            // Use floating-point inputs as is. Round arbitrary-precision
            // inputs to the specified precision.
            T t0 = C_<T>(0), t1 = C_<T>(0);
            RoundInitial(tMin, tMax, t0, t1);
            T f0 = F(t0), f1 = F(t1);
            return operator()(F, t0, t1, f0, f1, tRoot, fAtTRoot);
        }

        // Use this function when fAtTMin = F(tMin) and fAtTMax = F(tMax) are
        // already known. This is useful when |fAtTMin| or |fAtTMax| is
        // infinite, whereby you can pass sign(fAtTMin) or sign(fAtTMax)
        // rather than then an infinity because the bisector cares only about
        // the signs of F(t).
        bool operator()(std::function<T(T const&)> const& F,
            T const& tMin, T const& tMax, T const& fMin, T const& fMax,
            T& tRoot, T& fRoot)
        {
            GTL_ARGUMENT_ASSERT(
                tMin < tMax,
                "Invalid ordering of t-interval endpoints.");

            mFinalTMin = tMin;
            mFinalTMax = tMax;
            mFinalFMin = fMin;
            mFinalFMax = fMax;

            int32_t signFMin = (fMin > C_<T>(0) ? +1 : (fMin < C_<T>(0) ? -1 : 0));
            if (signFMin == 0)
            {
                mNumIterations = 0;
                tRoot = tMin;
                fRoot = C_<T>(0);
                return true;
            }

            int32_t signFMax = (fMax > C_<T>(0) ? +1 : (fMax < C_<T>(0) ? -1 : 0));
            if (signFMax == 0)
            {
                mNumIterations = 0;
                tRoot = tMax;
                fRoot = C_<T>(0);
                return true;
            }

            if (signFMin == signFMax)
            {
                // It is unknown whether the interval contains a root.
                mNumIterations = std::numeric_limits<size_t>::max();
                tRoot = C_<T>(0);
                fRoot = C_<T>(0);
                return false;
            }

            // The bisection steps.
            for (mNumIterations = 1; mNumIterations < mMaxIterations; ++mNumIterations)
            {
                // Use the floating-point average as is. Round the
                // arbitrary-precision average to the specified precision.
                tRoot = RoundAverage(mFinalTMin, mFinalTMax);
                fRoot = F(tRoot);

                // If the function is exactly zero, a root is found. For
                // fixed precision, the average of two consecutive numbers
                // might one of the current interval endpoints.
                int32_t signFRoot = (fRoot > C_<T>(0) ? +1 : (fRoot < C_<T>(0) ? -1 : 0));
                if (signFRoot != 0)
                {
                    if (tRoot != mFinalTMin && tRoot != mFinalTMax)
                    {
                        // Update the correct endpoint to the midpoint.
                        if (signFRoot == signFMin)
                        {
                            mFinalTMin = tRoot;
                            mFinalFMin = fRoot;
                        }
                        else // signFRoot == signFMax
                        {
                            mFinalTMax = tRoot;
                            mFinalFMax = fRoot;
                        }
                    }
                    else
                    {
                        // The current estimate is between two consecutive
                        // numbers of the specified precision.
                        break;
                    }
                }
                else
                {
                    // The function is exactly 0.
                    break;
                }
            }
            return true;
        }

        // Access to the state information of operator()(...).
        inline size_t GetNumIterations() const
        {
            return mNumIterations;
        }

        inline T const& GetFinalTMin() const
        {
            return mFinalTMin;
        }

        inline T const& GetFinalTMax() const
        {
            return mFinalTMax;
        }

        inline T const& GetFinalFMin() const
        {
            return mFinalFMin;
        }

        inline T const& GetFinalFMax() const
        {
            return mFinalFMax;
        }

    private:
        // Floating-point numbers are used without rounding.
        template <typename Numeric = T, IsFPType<Numeric> = 0>
        void RoundInitial(T const& inT0, T const& inT1, T& t0, T& t1)
        {
            t0 = inT0;
            t1 = inT1;
        }

        template <typename Numeric = T, IsFPType<Numeric> = 0>
        T RoundAverage(T const& t0, T const& t1)
        {
            T average = C_<T>(1, 2) * (t0 + t1);
            return average;
        }

        // Arbitrary-precision numbers are used with rounding.
        template <typename Numeric = T, IsAPType<Numeric> = 0>
        void RoundInitial(T const& inT0, T const& inT1, T& t0, T& t1)
        {
            if (mPrecision < std::numeric_limits<uint32_t>::max())
            {
                Convert(inT0, mPrecision, APRoundingMode::TO_NEAREST, t0);
                Convert(inT1, mPrecision, APRoundingMode::TO_NEAREST, t1);
            }
            else
            {
                t0 = inT0;
                t1 = inT1;
            }
        }

        template <typename Numeric = T, IsAPType<Numeric> = 0>
        T RoundAverage(T const& t0, T const& t1)
        {
            T average = std::ldexp(t0 + t1, -1);  // = (t0 + t1) / 2
            if (mPrecision < std::numeric_limits<uint32_t>::max())
            {
                T roundedAverage = C_<T>(0);
                Convert(average, mPrecision, APRoundingMode::TO_NEAREST, roundedAverage);
                return roundedAverage;
            }
            else
            {
                return average;
            }
        }

        size_t mMaxIterations, mPrecision;

        // State information for the operator()(...) functions.
        size_t mNumIterations;
        T mFinalTMin, mFinalTMax, mFinalFMin, mFinalFMax;

        friend class UnitTestRootsBisection1;
    };
}
