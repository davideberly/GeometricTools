// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.26

#pragma once

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

            // Test whether the closest line point is on the segment.
            if (lcResult.numClosestPairs == 2)
            {
                // The line intersects the circle in 2 points. If the segment
                // connecting the intersection points does not overlap the
                // input segment, then a segment endpoint is the closest point
                // on the segment to the circle. Moreover, the segment does
                // not intersect the circle even though the line does.
                Update(segment, circle, lcResult);
            }
            else
            {
                // The line does not intersect the circle or is tangent to the
                // circle. If the closest point on the line has a parameter
                // not in [0,1], then a segment endpoint is the closest point
                // on the segment to the circle. Moreover, the segment does
                // not intersect the circle even though the line does.
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
                // segment.p[1] is the closest point to the circle.
                Update(segment.p[1], one, circle, lcResult);
            }
            else if (t1 < zero)
            {
                // segment.p[0] is the closest point to the circle.
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
                // The segment is strictly inside the circle. Remove both
                // the t0-point and the t1-point. The closest segment endpoint
                // to the circle is the closest endpoint. Possibly both
                // segment endpoints are closest.
                auto pcResult0 = DCPQuery<T, Vector2<T>, Circle2<T>>{}(segment.p[0], circle);
                auto pcResult1 = DCPQuery<T, Vector2<T>, Circle2<T>>{}(segment.p[1], circle);
                if (pcResult0.distance < pcResult1.distance)
                {
                    // The endpoint segment.p[0] is closer to the circle than
                    // the endpoint segment.p[1].
                    lcResult.distance = pcResult0.distance;
                    lcResult.sqrDistance = pcResult0.sqrDistance;
                    lcResult.numClosestPairs = 1;
                    lcResult.parameter[0] = zero;
                    lcResult.parameter[1] = zero;
                    lcResult.closest[0][0] = pcResult0.closest[0];
                    lcResult.closest[0][1] = pcResult0.closest[1];
                    lcResult.closest[1][0] = { zero, zero };
                    lcResult.closest[1][1] = { zero, zero };
                }
                else if (pcResult0.distance > pcResult1.distance)
                {
                    // The endpoint segment.p[1] is closer to the circle than
                    // the endpoint segment.p[0].
                    lcResult.distance = pcResult1.distance;
                    lcResult.sqrDistance = pcResult1.sqrDistance;
                    lcResult.numClosestPairs = 1;
                    lcResult.parameter[0] = one;
                    lcResult.parameter[1] = zero;
                    lcResult.closest[0][0] = pcResult1.closest[0];
                    lcResult.closest[0][1] = pcResult1.closest[1];
                    lcResult.closest[1][0] = { zero, zero };
                    lcResult.closest[1][1] = { zero, zero };
                }
                else
                {
                    // The endpoints segment.p[0] and segment.p[1] are
                    // equidistant from the circle.
                    lcResult.distance = pcResult0.distance;
                    lcResult.sqrDistance = pcResult0.sqrDistance;
                    lcResult.numClosestPairs = 2;
                    lcResult.parameter[0] = zero;
                    lcResult.parameter[1] = one;
                    lcResult.closest[0][0] = pcResult0.closest[0];
                    lcResult.closest[0][1] = pcResult0.closest[1];
                    lcResult.closest[1][0] = pcResult1.closest[0];
                    lcResult.closest[1][1] = pcResult1.closest[1];
                }
            }
            else  // 0 <= t0 <= 1 && 0 <= t1 <= 1
            {
                // The line-circle intersection points are contained by the
                // segment.
                return;
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
