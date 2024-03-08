// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.08.20

#pragma once

// The reparameterization by arclength of a curve can be used for moving along
// a curve at constant speed. The documentation for the algorithms is
// https://www.geometrictools.com/Documentation/MovingAlongCurveSpecifiedSpeed.pdf

#include <Mathematics/Logger.h>
#include <Mathematics/ParametricCurve.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>

namespace gte
{
    template <int32_t N, typename T>
    class ReparameterizeByArclength
    {
    public:
        // The GTE interface for ParametricCurve<N,T> already contains the
        // support for computing arclength s from t. It has support for the
        // inversion (compute t from s), but the code here that uses the
        // hybrid Newton's method and bisection here will eventually replace
        // the GTE code and appear in the GTL distribution when it is shipped.
        ReparameterizeByArclength(std::shared_ptr<ParametricCurve<N, T>> const& curve)
            :
            mCurve(curve),
            mTMin(curve ? curve->GetTMin() : static_cast<T>(0)),
            mTMax(curve ? curve->GetTMax() : static_cast<T>(0)),
            mTotalArclength(curve ? curve->GetTotalLength() : static_cast<T>(0))
        {
            LogAssert(curve != nullptr, "The input curve must exist.");
        }

        // The output object stores the curves t-parameter corresponding to a
        // user-specified arclength s or a fraction r. The t-member stores the
        // t-parameter. The f-member is output.f = F(output.t, s). The member
        // output.numIterations is the number of iterations used to compute t
        // for the corresponding s or r.
        struct Output
        {
            Output()
                :
                t(static_cast<T>(0)),
                f(static_cast<T>(0)),
                numIterations(0)
            {
            }

            Output(T inT, T inF, size_t inIterations)
                :
                t(inT),
                f(inF),
                numIterations(inIterations)
            {
            }

            T t, f;
            size_t numIterations;
        };

        // Given an arclength s in [0,L] where the total arclength of the
        // curve is L = Arclength(tMin,tMax)), the function returns the
        // root t for F(t,s) = Arclength(tMin,t) - s. Set 'useBisection'
        // to 'true' to use bisection only. Set it to 'false' to use the
        // hybrid of Newton's method and bisection.
        Output GetT(T const& s, bool useBisection) const
        {
            // Clamp the input to the valid interval.
            T const zero = static_cast<T>(0);
            if (s <= zero)
            {
                return Output(mTMin, zero, 0);
            }

            if (s >= mTotalArclength)
            {
                return Output(mTMax, zero, 0);
            }

            // Compute a t-root of F(t, s) for the specified s-value. We know
            // that F(mTMin) < 0 and F(mTMax) > 0. Rather than use the initial
            // interval [mTMin,mTMax], choose a subinterval using an initial
            // guess for the t-root.
            T tMin = mTMin;
            T tMax = mTMax;
            T tMid = tMin + (tMax - tMin) * (s / mTotalArclength);
            T fMid = F(tMid, s);
            if (fMid > zero)
            {
                tMax = tMid;
            }
            else
            {
                tMin = tMid;
            }

            if (useBisection)
            {
                return DoBisection(tMin, tMax, s);
            }
            else
            {
                return DoNewtonsMethod(tMin, tMax, tMid, s);
            }
        }

        inline std::shared_ptr<ParametricCurve<N, T>> const& GetCurve() const
        {
            return mCurve;
        }

        inline T const& GetTMin() const
        {
            return mTMin;
        }

        inline T const& GetTMax() const
        {
            return mTMax;
        }

        inline T const& GetTotalArclength() const
        {
            return mTotalArclength;
        }

    private:
        // Choose maxIterations sufficiently large for convergence. The value
        // 4096 is sufficient. In practice, the number of iterations for type
        // 'float' is no larger than approximately 24 and for type 'double'
        // is no larger than approximately 53.
        static size_t constexpr maxIterations = 4096;

        T F(T const& t, T const& s) const
        {
            return mCurve->GetLength(mTMin, t) - s;
        }

        T DFDT(T const& t) const
        {
            return mCurve->GetSpeed(t);
        }

        bool BisectionConverged(T const& tMin, T const& tMax, T const& s, T& tMid, T& fMid) const
        {
            if (tMid == tMin || tMid == tMax)
            {
                // The precision of type T is such that tMin and tMax are
                // consecutive floating-point numbers. Their average cannot
                // be a floating-point number strictly between them. This is
                // the best you can do using type T. Return the t-endpoint
                // whose f-value has smaller magnitude.
                T fMin = F(tMin, s);
                T fMax = F(tMax, s);
                if (fMin <= fMax)
                {
                    tMid = tMin;
                    fMid = fMin;
                }
                else
                {
                    tMid = tMax;
                    fMid = fMax;
                }
                return true;
            }
            return false;
        }

        Output DoBisection(T tMin, T tMax, T const& s) const
        {
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);

            T tMid{}, fMid{};
            size_t numIterations{};
            for (numIterations = 1; numIterations <= maxIterations; ++numIterations)
            {
                // Compute the t-midpoint and the corresponding f-value. Exit
                // early if the f-value is zero.
                tMid = half * (tMin + tMax);
                fMid = F(tMid, s);
                if (fMid == zero)
                {
                    break;
                }

                // Convergence occurs when tMid is tMin or tMax.
                if (BisectionConverged(tMin, tMax, s, tMid, fMid))
                {
                    break;
                }

                // Update the correct t-endpoint using the t-midpoint.
                if (fMid > zero)
                {
                    tMax = tMid;
                }
                else
                {
                    tMin = tMid;
                }
            }

            return Output(tMid, fMid, numIterations);
        }

        Output DoNewtonsMethod(T tMin, T tMax, T tMid, T const& s) const
        {
            T const zero = static_cast<T>(0);
            T const half = static_cast<T>(0.5);

            // Store the iterates from Newton's method in order to determine
            // whether a cycle has occurs. If it does, further iterates will
            // already be in the set, so the function should return when a
            // cycle is detected.
            std::set<T> tIterates{};

            T fMid{};
            size_t numIterations{};
            for (numIterations = 1; numIterations <= maxIterations; ++numIterations)
            {
                // Test whether tMid is an iterate visited previously. If so,
                // a cycle has occurred.
                if (tIterates.insert(tMid).second == false)
                {
                    break;
                }

                // Evaluate F(tMid). Exit early if it is zero.
                fMid = F(tMid, s);
                if (fMid == zero)
                {
                    break;
                }

                // Update the bisection interval knowing the sign of F(tMid).
                // The current tMid becomes an endpoint of this interval.
                if (fMid > zero)
                {
                    tMax = tMid;
                }
                else
                {
                    tMin = tMid;
                }

                // Evaluate F'(tMid) >= 0. A bisection step must be taken when
                // F'(tMid) = 0 to avoid the division by zero.
                T dfdt = DFDT(tMid);
                if (dfdt == zero)
                {
                    // Division by zero is not allowed. Try the bisection step.
                    tMid = half * (tMin + tMax);
                    if (BisectionConverged(tMin, tMax, s, tMid, fMid))
                    {
                        break;
                    }
                }

                T tNext = tMid - fMid / dfdt;
                if (tNext == tMid)
                {
                    // The precision of type T is not large enough to
                    // disambiguate tMid and tNext. This is the best you can
                    // do using type T.
                    break;
                }

                // Determine whether to accept the Newton step or take the
                // bisection step.
                tMid = tNext;
                if (tMid < tMin || tMid > tMax)
                {
                    // The iterate is outside the root-bounding interval. Try
                    // the bisection step.
                    tMid = half * (tMin + tMax);
                    if (BisectionConverged(tMin, tMax, s, tMid, fMid))
                    {
                        break;
                    }
                }
            }

            return Output(tMid, fMid, numIterations);
        }

        // The curve is X(t) for t in [tMin,tMax]. The domain is specified in
        // the construction of the input 'curve' to the constructor.
        std::shared_ptr<ParametricCurve<N, T>> mCurve;
        T const mTMin;
        T const mTMax;

        // The length of the curve, L = Arclength(tMin,tMax). The arclength
        // itself is estimated by numerical integration of the speed function
        // for the curve, namely, |X'(t)|.
        T const mTotalArclength;
    };
}
