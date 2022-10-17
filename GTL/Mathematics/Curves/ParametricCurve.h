// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.10.15

#pragma once

// Inverse mapping of s = Length(t) given by t = Length^{-1}(s). The inverse
// length function generally cannot be written in closed form, in which case
// it is not directly computable. Instead, we can specify s and estimate the
// root t for F(t) = Length(t) - s. The derivative is F'(t) = Speed(t) >= 0,
// so F(t) is nondecreasing. To be robust, we use bisection to locate the
// root, although it is possible to use a hybrid of Newton's method and
// bisection. For details, see the document
// https://www.geometrictools.com/Documentation/MovingAlongCurveSpecifiedSpeed.pdf

#include <GTL/Mathematics/Algebra/Vector.h>
#include <GTL/Mathematics/Integration/IntgRomberg.h>
#include <GTL/Mathematics/RootFinders/RootsBisection1.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <vector>

namespace gtl
{
    template <typename T, size_t N>
    class ParametricCurve
    {
    protected:
        // Abstract base class for a parameterized curve X(t), where t is the
        // parameter in [tmin,tmax] and X is an N-tuple position. The first
        // constructor is for single-segment curves. The second constructor is
        // for multiple-segment curves. The times must be strictly increasing.
        ParametricCurve(T const& tmin, T const& tmax)
            :
            mRombergOrder(defaultRombergOrder),
            mMaxBisections(defaultMaxBisections),
            mPrecision(defaultPrecision),
            mTime(2),
            mSegmentLength(1, C_<T>(0)),
            mAccumulatedLength(1, C_<T>(0))
        {
            mTime[0] = tmin;
            mTime[1] = tmax;
        }

        ParametricCurve(size_t numSegments, T const* times)
            :
            mRombergOrder(defaultRombergOrder),
            mMaxBisections(defaultMaxBisections),
            mPrecision(defaultPrecision),
            mTime(numSegments + 1),
            mSegmentLength(numSegments, C_<T>(0)),
            mAccumulatedLength(numSegments, C_<T>(0))
        {
            GTL_ARGUMENT_ASSERT(
                numSegments > 0 && times != nullptr,
                "Invalid input to ParametricCurve constructor.");

            std::copy(times, times + numSegments + 1, mTime.begin());
        }

    public:
        virtual ~ParametricCurve() = default;

        // Member access.
        inline T const& GetTMin() const
        {
            return mTime.front();
        }

        inline T const& GetTMax() const
        {
            return mTime.back();
        }

        inline std::vector<T> const& GetTimes() const
        {
            return mTime;
        }

        inline size_t GetNumSegments() const
        {
            return mSegmentLength.size();
        }

        // Parameters used in GetLength(...), GetTotalLength() and
        // GetTime(...).

        // The default value is 8.
        inline void SetRombergOrder(size_t order)
        {
            mRombergOrder = std::max(order, static_cast<size_t>(1));
        }

        inline size_t GetRombergOrder() const
        {
            return mRombergOrder;
        }

        // The default value is 1024.
        inline void SetMaxBisections(size_t maxBisections)
        {
            mMaxBisections = std::max(maxBisections, static_cast<size_t>(1));
        }

        inline size_t GetMaxBisections() const
        {
            return mMaxBisections;
        }

        // The default value is 64.
        inline void SetPrecision(size_t precision)
        {
            mPrecision = std::max(precision, static_cast<size_t>(1));
        }

        inline size_t GetPrecision() const
        {
            return mPrecision;
        }

        // Evaluation of the curve. If you want only the position, pass in
        // order of 0. If you want the position and first derivative, pass in
        // order of 1, and so on. The output array 'jet' must have enough
        // storage to support the specified order. The values are ordered as:
        // position, first derivative, second derivative, and so on.
        virtual void Evaluate(T const& t, size_t order, Vector<T, N>* jet) const = 0;

        // Return the evaluation as an array of N-tuples of T values.
        void Evaluate(T const& t, size_t order, T* values) const
        {
            Evaluate(t, order, reinterpret_cast<Vector<T, N>*>(values));
        }

        // Differential geometric quantities.
        Vector<T, N> GetPosition(T const& t) const
        {
            Vector<T, N> position{};
            Evaluate(t, 0, &position);
            return position;
        }

        Vector<T, N> GetTangent(T const& t) const
        {
            std::array<Vector<T, N>, 2> jet{};  // (position, tangent)
            Evaluate(t, 1, jet.data());
            Normalize(jet[1]);
            return jet[1];
        }

        T GetSpeed(T const& t) const
        {
            std::array<Vector<T, N>, 2> jet{};  // (position, tangent)
            Evaluate(t, 1, jet.data());
            return Length(jet[1]);
        }

        T GetLength(T const& t0, T const& t1) const
        {
            std::function<T(T const&)> speed = [this](T const& t)
            {
                return GetSpeed(t);
            };

            if (mSegmentLength[0] == C_<T>(0))
            {
                // Lazy initialization of lengths of segments.
                T accumulated = C_<T>(0);
                for (size_t i = 0; i < mSegmentLength.size(); ++i)
                {
                    mSegmentLength[i] = IntgRomberg<T>::Integrate(
                        mRombergOrder, mTime[i], mTime[i + 1], speed);
                    accumulated += mSegmentLength[i];
                    mAccumulatedLength[i] = accumulated;
                }
            }

            T time0 = std::max(t0, GetTMin());
            T time1 = std::min(t1, GetTMax());
            auto iter0 = std::lower_bound(mTime.begin(), mTime.end(), time0);
            auto index0 = std::distance(mTime.begin(), iter0);
            auto iter1 = std::lower_bound(mTime.begin(), mTime.end(), time1);
            auto index1 = std::distance(mTime.begin(), iter1);

            T length{};
            if (index0 < index1)
            {
                length = C_<T>(0);
                if (t0 < *iter0)
                {
                    length += IntgRomberg<T>::Integrate(
                        mRombergOrder, time0, mTime[index0], speed);
                }

                size_t isup;
                if (t1 < *iter1)
                {
                    length += IntgRomberg<T>::Integrate(
                        mRombergOrder, mTime[index1 - 1], time1, speed);
                    isup = index1 - 1;
                }
                else
                {
                    isup = index1;
                }
                for (size_t i = index0; i < isup; ++i)
                {
                    length += mSegmentLength[i];
                }
            }
            else
            {
                length = IntgRomberg<T>::Integrate(
                    mRombergOrder, time0, time1, speed);
            }
            return length;
        }

        T GetTotalLength() const
        {
            // On-demand evaluation of the accumulated length array.
            if (mAccumulatedLength.back() != C_<T>(0))
            {
                return mAccumulatedLength.back();
            }
            else
            {
                return GetLength(mTime.front(), mTime.back());
            }
        }

        // See the comments at the beginning of this file about computing the
        // t-parameter from arc length.
        T GetTime(T const& length) const
        {
            if (length > C_<T>(0))
            {
                if (length < GetTotalLength())
                {
                    std::function<T(T)> F = [this, &length](T t)
                    {
                        return IntgRomberg<T>::Integrate(
                            mRombergOrder, mTime.front(), t,
                            [this](T const& z) { return GetSpeed(z); })
                            - length;
                    };

                    // We know that F(tmin) < 0 and F(tmax) > 0, which allows
                    // us to use bisection. Rather than bisect the entire
                    // interval, choose a reasonable guess for the initial
                    // interval.
                    RootsBisection1<T> bisector(mMaxBisections, mPrecision);
                    T ratio = length / GetTotalLength();
                    T omratio = C_<T>(1) - ratio;
                    T tmid = omratio * mTime.front() + ratio * mTime.back();
                    T fmid = F(tmid);
                    if (fmid > C_<T>(0))
                    {
                        bisector(F, mTime.front(), tmid, -C_<T>(1), C_<T>(1), tmid, fmid);
                    }
                    else if (fmid < C_<T>(0))
                    {
                        bisector(F, tmid, mTime.back(), -C_<T>(1), C_<T>(1), tmid, fmid);
                    }
                    return tmid;
                }
                else
                {
                    return mTime.back();
                }
            }
            else
            {
                return mTime.front();
            }
        }

        // Compute a subset of curve points according to the specified
        // attribute. The input 'numPoints' must be two or larger.
        void SubdivideByTime(size_t numPoints, Vector<T, N>* points) const
        {
            T delta = (mTime.back() - mTime.front()) / static_cast<T>(numPoints - 1);
            T time = mTime.front();
            for (size_t i = 0; i < numPoints; ++i)
            {
                points[i] = GetPosition(time);
                time += delta;
            }
        }

        void SubdivideByLength(size_t numPoints, Vector<T, N>* points) const
        {
            T delta = GetTotalLength() / static_cast<T>(numPoints - 1);
            T length = C_<T>(0);
            for (size_t i = 0; i < numPoints; ++i)
            {
                points[i] = GetPosition(GetTime(length));
                length += delta;
            }
        }

    protected:
        static size_t constexpr defaultRombergOrder = 8;
        static size_t constexpr defaultMaxBisections = 1024;
        static size_t constexpr defaultPrecision = 64;

        size_t mRombergOrder;
        size_t mMaxBisections;
        size_t mPrecision;
        std::vector<T> mTime;
        mutable std::vector<T> mSegmentLength;
        mutable std::vector<T> mAccumulatedLength;
    };
}
