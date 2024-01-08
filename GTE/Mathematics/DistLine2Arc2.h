// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.09.10

#pragma once

// Compute the distance between a line and an arc in 2D.
//
// The line is P + t * D, where P is a point on the line and D is not required
// to be unit length. The t-value is any real number.
//
// The circle containing the arc has center C and radius r. The arc has two
// endpoints E0 and E1 on the circle so that E1 is obtained from E0 by
// traversing counterclockwise. The application is responsible for ensuring
// that E0 and E1 are on the circle and that they are properly ordered.
//
// The number of pairs of closest points is result.numClosestPairs which is
// 1 or 2. If result.numClosestPairs is 1, result.parameter[0] is the line
// t-value for its closest point result.closest[0][0]. The arc closest point
// is result.closest[0][1]. If result.numClosestPairs is 2,
// result.parameter[0] and result.parameter[1] are the line t-values for its
// closest points result.closest[0][0] and result.closest[1][0]. The arc
// closest points are result.closest[0][1] and result.closest[1][1].

#include <Mathematics/DistLine2Circle2.h>
#include <Mathematics/DistPointLine.h>
#include <Mathematics/Arc2.h>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line2<T>, Arc2<T>>
    {
    public:
        using LCQuery = DCPQuery<T, Line2<T>, Circle2<T>>;
        using Result = typename LCQuery::Result;

        Result operator()(Line2<T> const& line, Arc2<T> const& arc)
        {
            Result result{};

            // Execute the query for line-circle. Test whether the circle
            // closest points are on or off the arc. If any closest point is
            // on the arc, there is no need to test arc endpoints for
            // closeness.
            Circle2<T> circle(arc.center, arc.radius);
            auto lcResult = DCPQuery<T, Line2<T>, Circle2<T>>{}(line, circle);
            for (size_t i = 0; i < lcResult.numClosestPairs; ++i)
            {
                if (arc.Contains(lcResult.closest[i][1]))
                {
                    size_t j = result.numClosestPairs++;
                    result.distance = lcResult.distance;
                    result.sqrDistance = lcResult.sqrDistance;
                    result.parameter[j] = lcResult.parameter[i];
                    result.closest[j][0] = lcResult.closest[i][0];
                    result.closest[j][1] = lcResult.closest[i][1];
                }
            }

            if (result.numClosestPairs > 0)
            {
                // At least one circle closest point is on the arc. There is
                // no need to test arc endpoints.
                return result;
            }

            // No circle closest points are on the arc. Compute distances to
            // the arc endpoints and select the minima.
            DCPQuery<T, Vector2<T>, Line2<T>> plQuery{};
            auto plResult0 = plQuery(arc.end[0], line);
            auto plResult1 = plQuery(arc.end[1], line);
            if (plResult0.sqrDistance < plResult1.sqrDistance)
            {
                result.distance = std::sqrt(plResult0.sqrDistance);
                result.sqrDistance = plResult0.sqrDistance;
                result.numClosestPairs = 1;
                result.parameter[0] = plResult0.parameter;
                result.closest[0][0] = plResult0.closest[1];
                result.closest[0][1] = arc.end[0];
            }
            else if (plResult1.sqrDistance < plResult0.sqrDistance)
            {
                result.distance = std::sqrt(plResult1.sqrDistance);
                result.sqrDistance = plResult1.sqrDistance;
                result.numClosestPairs = 1;
                result.parameter[0] = plResult1.parameter;
                result.closest[0][0] = plResult1.closest[1];
                result.closest[0][1] = arc.end[1];
            }
            else
            {
                result.distance = std::sqrt(plResult0.sqrDistance);
                result.sqrDistance = plResult0.sqrDistance;
                result.numClosestPairs = 2;
                result.parameter[0] = plResult0.parameter;
                result.parameter[1] = plResult1.parameter;
                result.closest[0][0] = plResult0.closest[1];
                result.closest[0][1] = arc.end[0];
                result.closest[1][0] = plResult1.closest[1];
                result.closest[1][1] = arc.end[1];
            }

            return result;
        }
    };
}
