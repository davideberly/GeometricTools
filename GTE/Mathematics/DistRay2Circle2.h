// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.08

#pragma once

#include <Mathematics/DistLine2Circle2.h>
#include <Mathematics/DistPoint2Circle2.h>
#include <Mathematics/Ray.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Ray2<T>, Circle2<T>>
    {
    public:
        using LCQuery = DCPQuery<T, Line2<T>, Circle2<T>>;
        using Result = typename LCQuery::Result;

        Result operator()(Ray2<T> const& ray, Circle2<T> const& circle)
        {
            // Execute the query for line-circle.
            T const zero = static_cast<T>(0);
            Line2<T> line(ray.origin, ray.direction);
            Result lcResult = LCQuery{}(line, circle);

            // Test whether the closest line point is on the ray.
            if (lcResult.numClosestPairs == 2)
            {
                // The line intersects the circle in 2 points. If the segment
                // connecting the intersection points does not overlap the
                // ray, then the ray origin is the closest point on the ray
                // to the circle. Moreover, the ray does not intersect the
                // circle even though the line does.
                if (lcResult.parameter[0] < zero && lcResult.parameter[1] < zero)
                {
                    Update(ray.origin, circle, lcResult);
                }
            }
            else
            {
                // The line does not intersect the circle or is tangent to the
                // circle. If the closest point on the line has a negative
                // parameter, then the ray origin is the closest point on the
                // ray to the circle. Moreover, the ray does not intersect the
                // circle even though the line does.
                if (lcResult.parameter[0] < zero)
                {
                    Update(ray.origin, circle, lcResult);
                }
            }

            return lcResult;
        }

    private:
        static void Update(Vector2<T> const& origin, Circle2<T> const& circle, Result& lcResult)
        {
            // Compute the closest circle point to the ray origin.
            T const zero = static_cast<T>(0);
            auto pcResult = DCPQuery<T, Vector2<T>, Circle2<T>>{}(origin, circle);

            // Update the line-circle result for the ray origin. The ray does
            // not intersect the circle even though the line does.
            lcResult.distance = pcResult.distance;
            lcResult.sqrDistance = pcResult.sqrDistance;
            lcResult.numClosestPairs = 1;
            lcResult.parameter[0] = zero;
            lcResult.parameter[1] = zero;
            lcResult.closest[0][0] = pcResult.closest[0];
            lcResult.closest[0][1] = pcResult.closest[1];
            lcResult.closest[1][0] = { zero, zero };
            lcResult.closest[1][1] = { zero, zero };
        }
    };
}
