// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.08.28

#pragma once

// Compute the distance between a ray and a circle in 2D. The circle is
// considered to be a curve, not a solid disk.
//
// The ray is P + t * D, where P is a point on the ray and D is not required
// to be unit length. The t-value satisfies t >= 0.
//
// The circle is C + r * U(s), where C is the center, r > 0 is the radius,
// and U(s) = (cos(s), sin(s)) for s in [0,2*pi).
//
// The number of pairs of closest points is result.numClosestPairs which is
// 1 or 2. If result.numClosestPairs is 1, result.parameter[0] is the ray
// t-value for its closest point result.closest[0][0]. The circle closest
// point is result.closest[0][1]. If result.numClosestPairs is 2,
// result.parameter[0] and result.parameter[1] are the ray t-values for its
// closest points result.closest[0][0] and result.closest[1][0]. The circle
// closest points are result.closest[0][1] and result.closest[1][1].

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

            // Restrict the analysis to ray-circle.
            if (lcResult.numClosestPairs == 2)
            {
                // The segment connecting the line-circle intersection points
                // has parameter interval [t0,t1]. Determine how this
                // intersects with the ray interval [0,+infinity) and modify
                // lcResult accordingly.
                Update(ray, circle, lcResult);
            }
            else  // lcResult.numClosestPairs = 1
            {
                // The line does not intersect the circle or is tangent to the
                // circle. If the closest line point to the circle has a
                // negative parameter, then the ray is outside the circle and
                // the ray origin is the closest ray point to the circle.
                if (lcResult.parameter[0] < zero)
                {
                    Update(ray.origin, circle, lcResult);
                }
            }

            return lcResult;
        }

    private:
        static void Update(Ray2<T> const& ray, Circle2<T> const& circle, Result& lcResult)
        {
            T const zero = static_cast<T>(0);
            auto const& t0 = lcResult.parameter[0];
            auto const& t1 = lcResult.parameter[1];

            if (t1 <= zero)
            {
                // The ray.origin is the closest point to the circle.
                Update(ray.origin, circle, lcResult);
            }
            else if (t0 < zero)
            {
                // The ray.origin is strictly inside the circle. Remove the
                // t0-point.
                lcResult.numClosestPairs = 1;
                lcResult.parameter[0] = lcResult.parameter[1];
                lcResult.parameter[1] = zero;
                lcResult.closest[0][0] = lcResult.closest[1][0];
                lcResult.closest[0][1] = lcResult.closest[1][1];
                lcResult.closest[1][0] = { zero, zero };
                lcResult.closest[1][1] = { zero, zero };
            }
            else  // 0 <= t0 < t1
            {
                // The line-circle intersection points are contained by the
                // ray.
            }
        }

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
