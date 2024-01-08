// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the triangle to be a solid.

#include <Mathematics/IntrIntervals.h>
#include <Mathematics/IntrLine2Triangle2.h>
#include <Mathematics/Segment.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename Real>
    class TIQuery<Real, Segment2<Real>, Triangle2<Real>>
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

        // The segment is P0 + t * (P1 - P0) for t in [0,1].
        Result operator()(Segment2<Real> const& segment, Triangle2<Real> const& triangle)
        {
            Result result{};
            FIQuery<Real, Segment2<Real>, Triangle2<Real>> stQuery{};
            result.intersect = stQuery(segment, triangle).intersect;
            return result;
        }
    };

    template <typename Real>
    class FIQuery <Real, Segment2<Real>, Triangle2<Real>>
        :
        public FIQuery<Real, Line2<Real>, Triangle2<Real>>
    {
    public:
        struct Result
            :
            public FIQuery<Real, Line2<Real>, Triangle2<Real>>::Result
        {
            Result()
                :
                FIQuery<Real, Line2<Real>, Triangle2<Real>>::Result{}
            {
            }

            // No additional information to compute.
        };

        // The segment is P0 + t * (P1 - P0) for t in [0,1].
        Result operator()(Segment2<Real> const& segment, Triangle2<Real> const& triangle)
        {
            Result result{};
            Vector2<Real> const& segOrigin = segment.p[0];
            Vector2<Real> segDirection = segment.p[1] - segment.p[0];
            DoQuery(segOrigin, segDirection, triangle, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = segOrigin + result.parameter[i] * segDirection;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector2<Real> const& origin, Vector2<Real> const& direction,
            Triangle2<Real> const& triangle, Result& result)
        {
            FIQuery<Real, Line2<Real>, Triangle2<Real>>::DoQuery(
                origin, direction, triangle, result);

            if (result.intersect)
            {
                // The line containing the segment intersects the triangle;
                // the t-interval is [t0,t1]. The segment intersects the
                // triangle as long as [t0,t1] overlaps the segment t-interval
                // [0,1].
                FIQuery<Real, std::array<Real, 2>, std::array<Real, 2>> iiQuery{};
                std::array<Real, 2> segInterval{ static_cast<Real>(0), static_cast<Real>(1) };
                auto iiResult = iiQuery(result.parameter, segInterval);
                if (iiResult.intersect)
                {
                    result.numIntersections = iiResult.numIntersections;
                    result.parameter = iiResult.overlap;
                }
                else
                {
                    // The line containing the segment does not intersect the
                    // triangle.
                    result = Result{};
                }
            }
        }
    };
}
