// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the circle to be a solid (disk).

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Circle2.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment2<T>, Circle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false)
            {
            }

            bool intersect;
        };

        Result operator()(Segment2<T> const& segment, Circle2<T> const& circle)
        {
            Result result{};
            FIQuery<T, Segment2<T>, Circle2<T>> scQuery{};
            result.intersect = scQuery(segment, circle).intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment2<T>, Circle2<T>>
        :
        public FIQuery<T, Line2<T>, Circle2<T>>
    {
    public:
        struct Result
            :
            public FIQuery<T, Line2<T>, Circle2<T>>::Result
        {
            Result()
                :
                FIQuery<T, Line2<T>, Circle2<T>>::Result{}
            {
            }

            // No additional information to compute.
        };

        Result operator()(Segment2<T> const& segment, Circle2<T> const& circle)
        {
            Vector2<T> segOrigin{}, segDirection{};
            T segExtent{};
            segment.GetCenteredForm(segOrigin, segDirection, segExtent);

            Result result{};
            DoQuery(segOrigin, segDirection, segExtent, circle, result);
            for (int32_t i = 0; i < result.numIntersections; ++i)
            {
                result.point[i] = segOrigin + result.parameter[i] * segDirection;
            }
            return result;
        }

    protected:
        void DoQuery(Vector2<T> const& segOrigin,
            Vector2<T> const& segDirection, T segExtent,
            Circle2<T> const& circle, Result& result)
        {
            FIQuery<T, Line2<T>, Circle2<T>>::DoQuery(segOrigin,
                segDirection, circle, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the disk; the
                // t-interval is [t0,t1].  The segment intersects the disk as
                // long as [t0,t1] overlaps the segment t-interval
                // [-segExtent,+segExtent].
                std::array<T, 2> segInterval = { -segExtent, segExtent };
                FIQuery<T, std::array<T, 2>, std::array<T, 2>> iiQuery{};
                auto iiResult = iiQuery(result.parameter, segInterval);
                result.intersect = iiResult.intersect;
                result.numIntersections = iiResult.numIntersections;
                result.parameter = iiResult.overlap;
            }
        }
    };
}
