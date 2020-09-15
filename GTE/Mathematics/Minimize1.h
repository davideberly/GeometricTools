// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Math.h>
#include <algorithm>
#include <functional>

// The interval [t0,t1] provided to GetMinimum(Real,Real,Real,Real&,Real&)
// is processed by examining subintervals.  On each subinteral [a,b], the
// values f0 = F(a), f1 = F((a+b)/2), and f2 = F(b) are examined.  If
// {f0,f1,f2} is monotonic, then [a,b] is subdivided and processed.  The
// maximum depth of the recursion is limited by 'maxLevel'.  If {f0,f1,f2}
// is not monotonic, then two cases arise.  First, if f1 = min{f0,f1,f2},
// then {f0,f1,f2} is said to "bracket a minimum" and GetBracketedMinimum(*)
// is called to locate the function minimum.  The process uses a form of
// bisection called "parabolic interpolation" and the maximum number of
// bisection steps is 'maxBracket'.  Second, if f1 = max{f0,f1,f2}, then
// {f0,f1,f2} brackets a maximum.  The minimum search continues recursively
// as before on [a,(a+b)/2] and [(a+b)/2,b].

namespace gte
{
    template <typename Real>
    class Minimize1
    {
    public:
        // Construction.
        Minimize1(std::function<Real(Real)> const& F, int maxLevel, int maxBracket,
            Real epsilon = (Real)1e-08, Real tolerance = (Real)1e-04)
            :
            mFunction(F),
            mMaxLevel(maxLevel),
            mMaxBracket(maxBracket),
            mEpsilon(0),
            mTolerance(0)
        {
            SetEpsilon(epsilon);
            SetTolerance(tolerance);
        }

        // Member access.
        inline void SetEpsilon(Real epsilon)
        {
            mEpsilon = (epsilon > (Real)0 ? epsilon : (Real)0);
        }

        inline void SetTolerance(Real tolerance)
        {
            mTolerance = (tolerance > (Real)0 ? tolerance : (Real)0);
        }

        inline Real GetEpsilon() const
        {
            return mEpsilon;
        }

        inline Real GetTolerance() const
        {
            return mTolerance;
        }

        // Search for a minimum of 'function' on the interval [t0,t1] using an
        // initial guess of 'tInitial'.  The location of the minimum is 'tMin'
        // and/ the value of the minimum is 'fMin'.
        void GetMinimum(Real t0, Real t1, Real tInitial, Real& tMin, Real& fMin)
        {
            LogAssert(t0 <= tInitial && tInitial <= t1, "Invalid initial t value.");

            mTMin = std::numeric_limits<Real>::max();
            mFMin = std::numeric_limits<Real>::max();

            Real f0 = mFunction(t0);
            if (f0 < mFMin)
            {
                mTMin = t0;
                mFMin = f0;
            }

            Real fInitial = mFunction(tInitial);
            if (fInitial < mFMin)
            {
                mTMin = tInitial;
                mFMin = fInitial;
            }

            Real f1 = mFunction(t1);
            if (f1 < mFMin)
            {
                mTMin = t1;
                mFMin = f1;
            }

            GetMinimum(t0, f0, tInitial, fInitial, t1, f1, mMaxLevel);

            tMin = mTMin;
            fMin = mFMin;
        }

    private:
        // This is called to start the search on [t0,tInitial] and
        // [tInitial,t1].
        void GetMinimum(Real t0, Real f0, Real t1, Real f1, int level)
        {
            if (level-- == 0)
            {
                return;
            }

            Real tm = (Real)0.5 * (t0 + t1);
            Real fm = mFunction(tm);
            if (fm < mFMin)
            {
                mTMin = tm;
                mFMin = fm;
            }

            if (f0 - (Real)2 * fm + f1 > (Real)0)
            {
                // The quadratic fit has positive second derivative at the
                // midpoint.
                if (f1 > f0)
                {
                    if (fm >= f0)
                    {
                        // Increasing, repeat on [t0,tm].
                        GetMinimum(t0, f0, tm, fm, level);
                    }
                    else
                    {
                        // Not monotonic, have a bracket.
                        GetBracketedMinimum(t0, f0, tm, fm, t1, f1, level);
                    }
                }
                else if (f1 < f0)
                {
                    if (fm >= f1)
                    {
                        // Decreasing, repeat on [tm,t1].
                        GetMinimum(tm, fm, t1, f1, level);
                    }
                    else
                    {
                        // Not monotonic, have a bracket.
                        GetBracketedMinimum(t0, f0, tm, fm, t1, f1, level);
                    }
                }
                else
                {
                    // Constant, repeat on [t0,tm] and [tm,t1].
                    GetMinimum(t0, f0, tm, fm, level);
                    GetMinimum(tm, fm, t1, f1, level);
                }
            }
            else
            {
                // The quadratic fit has nonpositive second derivative at the
                // midpoint.
                if (f1 > f0)
                {
                    // Repeat on [t0,tm].
                    GetMinimum(t0, f0, tm, fm, level);
                }
                else if (f1 < f0)
                {
                    // Repeat on [tm,t1].
                    GetMinimum(tm, fm, t1, f1, level);
                }
                else
                {
                    // Repeat on [t0,tm] and [tm,t1].
                    GetMinimum(t0, f0, tm, fm, level);
                    GetMinimum(tm, fm, t1, f1, level);
                }
            }
        }

        // This is called recursively to search [a,(a+b)/2] and [(a+b)/2,b].
        void GetMinimum(Real t0, Real f0, Real tm, Real fm, Real t1, Real f1, int level)
        {
            if (level-- == 0)
            {
                return;
            }

            if ((t1 - tm) * (f0 - fm) > (tm - t0) * (fm - f1))
            {
                // The quadratic fit has positive second derivative at the
                // midpoint.
                if (f1 > f0)
                {
                    if (fm >= f0)
                    {
                        // Increasing, repeat on [t0,tm].
                        GetMinimum(t0, f0, tm, fm, level);
                    }
                    else
                    {
                        // Not monotonic, have a bracket.
                        GetBracketedMinimum(t0, f0, tm, fm, t1, f1, level);
                    }
                }
                else if (f1 < f0)
                {
                    if (fm >= f1)
                    {
                        // Decreasing, repeat on [tm,t1].
                        GetMinimum(tm, fm, t1, f1, level);
                    }
                    else
                    {
                        // Not monotonic, have a bracket.
                        GetBracketedMinimum(t0, f0, tm, fm, t1, f1, level);
                    }
                }
                else
                {
                    // Constant, repeat on [t0,tm] and [tm,t1].
                    GetMinimum(t0, f0, tm, fm, level);
                    GetMinimum(tm, fm, t1, f1, level);
                }
            }
            else
            {
                // The quadratic fit has a nonpositive second derivative at
                // the midpoint.
                if (f1 > f0)
                {
                    // Repeat on [t0,tm].
                    GetMinimum(t0, f0, tm, fm, level);
                }
                else if (f1 < f0)
                {
                    // Repeat on [tm,t1].
                    GetMinimum(tm, fm, t1, f1, level);
                }
                else
                {
                    // Repeat on [t0,tm] and [tm,t1].
                    GetMinimum(t0, f0, tm, fm, level);
                    GetMinimum(tm, fm, t1, f1, level);
                }
            }
        }

        // This is called when {f0,f1,f2} brackets a minimum.
        void GetBracketedMinimum(Real t0, Real f0, Real tm, Real fm, Real t1, Real f1, int level)
        {
            for (int i = 0; i < mMaxBracket; ++i)
            {
                // Update minimum value.
                if (fm < mFMin)
                {
                    mTMin = tm;
                    mFMin = fm;
                }

                // Test for convergence.
                if (std::fabs(t1 - t0) <= (Real)2 * mTolerance * std::fabs(tm) + mEpsilon)
                {
                    break;
                }

                // Compute vertex of interpolating parabola.
                Real dt0 = t0 - tm;
                Real dt1 = t1 - tm;
                Real df0 = f0 - fm;
                Real df1 = f1 - fm;
                Real tmp0 = dt0 * df1;
                Real tmp1 = dt1 * df0;
                Real denom = tmp1 - tmp0;
                if (std::fabs(denom) <= mEpsilon)
                {
                    return;
                }

                // Compute tv and clamp to [t0,t1] to offset floating-point
                // rounding errors.
                Real tv = tm + (Real)0.5 * (dt1 * tmp1 - dt0 * tmp0) / denom;
                tv = std::max(t0, std::min(tv, t1));
                Real fv = mFunction(tv);
                if (fv < mFMin)
                {
                    mTMin = tv;
                    mFMin = fv;
                }

                if (tv < tm)
                {
                    if (fv < fm)
                    {
                        t1 = tm;
                        f1 = fm;
                        tm = tv;
                        fm = fv;
                    }
                    else
                    {
                        t0 = tv;
                        f0 = fv;
                    }
                }
                else if (tv > tm)
                {
                    if (fv < fm)
                    {
                        t0 = tm;
                        f0 = fm;
                        tm = tv;
                        fm = fv;
                    }
                    else
                    {
                        t1 = tv;
                        f1 = fv;
                    }
                }
                else
                {
                    // The vertex of parabola is already at middle sample point.
                    GetMinimum(t0, f0, tm, fm, level);
                    GetMinimum(tm, fm, t1, f1, level);
                }
            }
        }

        std::function<Real(Real)> mFunction;
        int mMaxLevel;
        int mMaxBracket;
        Real mTMin, mFMin;
        Real mEpsilon, mTolerance;
    };
}
