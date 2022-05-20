// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.20

#pragma once

// This is an implementation of Brent's Method for computing a root of a
// function on an interval [t0,t1] for which F(t0)*F(t1) < 0. The method
// uses inverse quadratic interpolation to generate a root estimate but
// falls back to inverse linear interpolation (secant method) if
// necessary. Moreover, based on previous iterates, the method will fall
// back to bisection when it appears the interpolated estimate is not of
// sufficient quality.
//
//   maxIterations:
//       The maximum number of iterations used to locate a root.  This
//       should be positive.
//   negFTolerance, posFTolerance:
//       The root estimate t is accepted when the function value F(t)
//       satisfies negFTolerance <= F(t) <= posFTolerance.  The values
//       must satisfy:  negFTolerance <= 0, posFTolerance >= 0.
//   stepTTolerance:
//       Brent's Method requires additional tests before an interpolated
//       t-value is accepted as the next root estimate.  One of these
//       tests compares the difference of consecutive iterates and
//       requires it to be larger than a user-specified t-tolerance (to
//       ensure progress is made).  This parameter is that tolerance
//       and should be nonnegative.
//   convTTolerance:
//       The root search is allowed to terminate when the current
//       subinterval [tsub0,tsub1] is sufficiently small, say,
//       |tsub1 - tsub0| <= tolerance.  This parameter is that tolerance
//       and should be nonnegative.

#include <GTL/Mathematics/Arithmetic/Constants.h>
#include <GTL/Utility/Exceptions.h>
#include <algorithm>
#include <cmath>
#include <functional>

namespace gtl
{
    template <typename T>
    class RootsBrentsMethod
    {
    public:
        RootsBrentsMethod(size_t maxIterations, T const& negFTolerance,
            T const& posFTolerance, T const& stepTTolerance,
            T const& convTTolerance)
            :
            mMaxIterations(maxIterations),
            mNegFTolerance(negFTolerance),
            mPosFTolerance(posFTolerance),
            mStepTTolerance(stepTTolerance),
            mConvTTolerance(convTTolerance)
        {
            GTL_ARGUMENT_ASSERT(
                mMaxIterations > 0 &&
                mNegFTolerance <= C_<T>(0) &&
                mPosFTolerance >= C_<T>(0) &&
                mStepTTolerance >= C_<T>(0) &&
                mConvTTolerance >= C_<T>(0),
                "Invalid argument.");
        }

        // Use this function when F(tMin) and F(tMax) are not already known.
        size_t operator()(std::function<T(T const&)> const& F,
            T const& tMin, T const& tMax, T& tRoot, T& fAtTRoot)
        {
            GTL_ARGUMENT_ASSERT(
                tMin < tMax,
                "Invalid ordering of t-interval endpoints.");

            T fMin = F(tMin);
            T fMax = F(tMax);
            return operator()(F, tMin, tMax, fMin, fMax, tRoot, fAtTRoot);
        }

        // Use this function when fAtTMin = F(tMin) and fAtTMax = F(tMax) are
        // already known. This is useful when |fAtTMin| or |fAtTMax| is
        // infinite, whereby you can pass sign(fAtTMin) or sign(fAtTMax)
        // rather than then an infinity because the bisector cares only about
        // the signs of F(t).
        size_t operator()(std::function<T(T const&)> const& F,
            T const& tMin, T const& tMax, T const& fMin, T const& fMax,
            T& tRoot, T& fAtTRoot)
        {
            GTL_ARGUMENT_ASSERT(
                tMin < tMax,
                "Invalid ordering of t-interval endpoints.");

            if (mNegFTolerance <= fMin && fMin <= mPosFTolerance)
            {
                // This endpoint is an approximate root that satisfies the
                // function tolerance.
                tRoot = tMin;
                fAtTRoot = fMin;
                return 1;
            }

            if (mNegFTolerance <= fMax && fMax <= mPosFTolerance)
            {
                // This endpoint is an approximate root that satisfies the
                // function tolerance.
                tRoot = tMax;
                fAtTRoot = fMax;
                return 1;
            }

            if ((fMin > C_<T>(0) && fMax > C_<T>(0)) || (fMin < C_<T>(0) && fMax < C_<T>(0)))
            {
                // The input interval must bound a root.
                return 0;
            }

            T t0 = tMin, t1 = tMax, f0 = fMin, f1 = fMax;
            if (std::fabs(f0) < std::fabs(f1))
            {
                // Swap t0 and t1 so that |F(t1)| <= |F(t0)|. The number t1
                // is considered to be the best estimate of the root.
                std::swap(t0, t1);
                std::swap(f0, f1);
            }

            // Initialize values for the root search.
            T t2 = t0, t3 = t0, f2 = f0;
            bool prevBisected = true;

            // The root search.
            size_t iteration{};
            for (iteration = 2; iteration < mMaxIterations; ++iteration)
            {
                T fDiff01 = f0 - f1;
                T fDiff02 = f0 - f2;
                T fDiff12 = f1 - f2;
                T invFDiff01 = C_<T>(1) / fDiff01;
                T s;
                if (fDiff02 != C_<T>(0) && fDiff12 != C_<T>(0))
                {
                    // Use inverse quadratic interpolation.
                    T infFDiff02 = C_<T>(1) / fDiff02;
                    T invFDiff12 = C_<T>(1) / fDiff12;
                    s =
                        t0 * f1 * f2 * invFDiff01 * infFDiff02 -
                        t1 * f0 * f2 * invFDiff01 * invFDiff12 +
                        t2 * f0 * f1 * infFDiff02 * invFDiff12;
                }
                else
                {
                    // Use inverse linear interpolation (secant method).
                    s = (t1 * f0 - t0 * f1) * invFDiff01;
                }

                // Compute values need in the accept-or-reject tests.
                T tDiffSAvr = s - C_<T>(3, 4) * t0 - C_<T>(1, 4) * t1;
                T tDiffS1 = s - t1;
                T absTDiffS1 = std::fabs(tDiffS1);
                T absTDiff12 = std::fabs(t1 - t2);
                T absTDiff23 = std::fabs(t2 - t3);

                bool currBisected = false;
                if (tDiffSAvr * tDiffS1 > C_<T>(0))
                {
                    // The value s is not between 0.75*t0 + 0.25*t1 and t1.
                    // NOTE: The algorithm sometimes has t0 < t1 but sometimes
                    // t1 < t0, so the between-ness test does not use simple
                    // comparisons.
                    currBisected = true;
                }
                else if (prevBisected)
                {
                    // The first of Brent's tests to determine whether to
                    // accept the interpolated s-value.
                    currBisected =
                        (absTDiffS1 >= C_<T>(1, 2) * absTDiff12) ||
                        (absTDiff12 <= mStepTTolerance);
                }
                else
                {
                    // The second of Brent's tests to determine whether to
                    // accept the interpolated s-value.
                    currBisected =
                        (absTDiffS1 >= C_<T>(1, 2) * absTDiff23) ||
                        (absTDiff23 <= mStepTTolerance);
                }

                if (currBisected)
                {
                    // One of the additional tests failed, so reject the
                    // interpolated s-value and use bisection instead.
                    s = C_<T>(1, 2) * (t0 + t1);
                    if (s == t0 || s == t1)
                    {
                        // The numbers t0 and t1 are consecutive
                        // floating-point numbers.
                        tRoot = s;
                        fAtTRoot = F(tRoot);
                        return iteration;
                    }
                    prevBisected = true;
                }
                else
                {
                    prevBisected = false;
                }

                // Evaluate the function at the new estimate and test for
                // convergence.
                T fs = F(s);
                if (mNegFTolerance <= fs && fs <= mPosFTolerance)
                {
                    tRoot = s;
                    fAtTRoot = F(tRoot);
                    return iteration;
                }

                // Update the subinterval to include the new estimate as an
                // endpoint.
                t3 = t2;
                t2 = t1;
                f2 = f1;
                if (f0 * fs < C_<T>(0))
                {
                    t1 = s;
                    f1 = fs;
                }
                else
                {
                    t0 = s;
                    f0 = fs;
                }

                // Allow the algorithm to terminate when the subinterval is
                // sufficiently small.
                if (std::fabs(t1 - t0) <= mConvTTolerance)
                {
                    tRoot = t1;
                    fAtTRoot = F(tRoot);
                    return iteration;
                }

                // A loop invariant is that t1 is the root estimate,
                // F(t0)*F(t1) < 0 and |F(t1)| <= |F(t0)|.
                if (std::fabs(f0) < std::fabs(f1))
                {
                    std::swap(t0, t1);
                    std::swap(f0, f1);
                }
            }

            // Failed to converge in the specified number of iterations.
            return mMaxIterations;
        }

    private:
        size_t mMaxIterations;
        T mNegFTolerance;
        T mPosFTolerance;
        T mStepTTolerance;
        T mConvTTolerance;

        friend class UnitTestRootsBrentsMethod;
    };
}
