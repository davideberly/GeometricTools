// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.28

#pragma once

// Compute the distance between a segment and a circle in 2D. The circle is
// considered to be a curve, not a solid disk.
//
// The segment has endpoints P0 and P1 and is parameterized by
// P0 + t * (P1 - P0). The t-value satisfies 0 <= t <= 1.
//
// The circle is C + r * U(s), where C is the center, r > 0 is the radius,
// and U(s) = (cos(s), sin(s)) for s in [0,2*pi).
//
// The number of pairs of closest points is result.numClosestPairs which is
// 1 or 2. If result.numClosestPairs is 1, result.parameter[0] is the segment
// t-value for its closest point result.closest[0][0]. The circle closest
// point is result.closest[0][1]. If result.numClosestPairs is 2,
// result.parameter[0] and result.parameter[1] are the segment t-values for
// its closest points result.closest[0][0] and result.closest[1][0]. The
// circle closest points are result.closest[0][1] and result.closest[1][1].

#include <Mathematics/DistLine2Circle2.h>
#include <Mathematics/DistPoint2Circle2.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment2<T>, Circle2<T>>
    {
    public:
        using LCQuery = DCPQuery<T, Line2<T>, Circle2<T>>;
        using Result = typename LCQuery::Result;

        Result operator()(Segment2<T> const& segment, Circle2<T> const& circle)
        {
            // Execute the query for line-circle.
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Line2<T> line(segment.p[0], segment.p[1] - segment.p[0]);
            Result lcResult = LCQuery{}(line, circle);

            // Restrict the analysis to segment-circle.
            if (lcResult.numClosestPairs == 2)
            {
                // The segment connecting the line-circle intersection points
                // has parameter interval [t0,t1]. Determine how this
                // intersects with the segment interval [0,1] and modify
                // lcResult accordingly.
                Update(segment, circle, lcResult);
            }
            else
            {
                // The line does not intersect the circle or is tangent to the
                // circle. If the closest point on the line has a parameter
                // not in [0,1], then a segment is outside the circle and a
                // segment endpoint is the closest segment point segment to
                // the circle.
                if (lcResult.parameter[0] < zero)
                {
                    Update(segment.p[0], zero, circle, lcResult);
                }
                else if (lcResult.parameter[0] > one)
                {
                    Update(segment.p[1], one, circle, lcResult);
                }
            }

            return lcResult;
        }

    private:
        static void Update(Segment2<T> const& segment, Circle2<T> const& circle, Result& lcResult)
        {
            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            auto const& t0 = lcResult.parameter[0];
            auto const& t1 = lcResult.parameter[1];

            if (t0 > one)
            {
                // The segment.p[1] is the closest point to the circle.
                Update(segment.p[1], one, circle, lcResult);
            }
            else if (t1 < zero)
            {
                // The segment.p[0] is the closest point to the circle.
                Update(segment.p[0], zero, circle, lcResult);
            }
            else if (t0 < zero && t1 < one)
            {
                // The segment overlaps the t1-point. Remove the t0-point.
                lcResult.numClosestPairs = 1;
                lcResult.parameter[0] = lcResult.parameter[1];
                lcResult.parameter[1] = zero;
                lcResult.closest[0][0] = lcResult.closest[1][0];
                lcResult.closest[0][1] = lcResult.closest[1][1];
                lcResult.closest[1][0] = { zero, zero };
                lcResult.closest[1][1] = { zero, zero };
            }
            else if (t0 > zero && t1 > one)
            {
                // The segment overlaps the t0-point. Remove the t1-point.
                lcResult.numClosestPairs = 1;
                lcResult.parameter[1] = zero;
                lcResult.closest[1][0] = { zero, zero };
                lcResult.closest[1][1] = { zero, zero };
            }
            else if (t0 < zero && t1 > one)
            {
                // The segment is strictly inside the circle. Remove both the
                // t0-point and the t1-point.
                lcResult = Result{};
            }
            else  // 0 <= t0 < t1 <= 1
            {
                // The line-circle intersection points are contained by the
                // segment.
            }
        }

        static void Update(Vector2<T> const& endpoint, T const& parameter,
            Circle2<T> const& circle, Result& lcResult)
        {
            // Compute the closest circle point to the ray origin.
            T const zero = static_cast<T>(0);
            auto pcResult = DCPQuery<T, Vector2<T>, Circle2<T>>{}(endpoint, circle);

            // Update the line-circle result for the segment endpoint. The
            // segment does not intersect the circle even though the line
            // does.
            lcResult.distance = pcResult.distance;
            lcResult.sqrDistance = pcResult.sqrDistance;
            lcResult.numClosestPairs = 1;
            lcResult.parameter[0] = parameter;
            lcResult.parameter[1] = zero;
            lcResult.closest[0][0] = pcResult.closest[0];
            lcResult.closest[0][1] = pcResult.closest[1];
            lcResult.closest[1][0] = { zero, zero };
            lcResult.closest[1][1] = { zero, zero };
        }
    };
}
