// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2022.12.02

#pragma once

#include "DistPoint2Circle2.h"
#include <Mathematics/Arc2.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector2<T>, Arc2<T>>
    {
    public:
        // The input point is stored in the member closest[0]. If a single
        // point on the arc is closest to the input point, the member
        // closest[1] is set to that point and the equidistant member is set
        // to false. If the entire arc is equidistant to the point, the member
        // closest[1] is set to the endpoint E0 of the arc and the equidistant
        // member is set to true.
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                closest{ Vector2<T>::Zero(), Vector2<T>::Zero() },
                equidistant(false)
            {
            }

            T distance, sqrDistance;
            std::array<Vector2<T>, 2> closest;
            bool equidistant;
        };

        Result operator()(Vector2<T> const& point, Arc2<T> const& arc)
        {
            Result result{};

            Circle2<T> circle(arc.center, arc.radius);
            DCPQuery<T, Vector2<T>, Circle2<T>> pcQuery{};
            auto pcResult = pcQuery(point, circle);
            if (!pcResult.equidistant)
            {
                // Test whether the closest circle point is on the arc. If it
                // is, that point is the closest arc point. If it is not, the
                // closest arc point is an arc endpoint. Determine which
                // endpoint that is.
                if (arc.Contains(pcResult.closest[1]))
                {
                    result.distance = pcResult.distance;
                    result.sqrDistance = pcResult.sqrDistance;
                    result.closest = pcResult.closest;
                    result.equidistant = pcResult.equidistant;
                }
                else
                {
                    Vector2<T> diff0 = arc.end[0] - point;
                    Vector2<T> diff1 = arc.end[1] - point;
                    T sqrLength0 = Dot(diff0, diff0);
                    T sqrLength1 = Dot(diff1, diff1);
                    if (sqrLength0 <= sqrLength1)
                    {
                        result.distance = std::sqrt(sqrLength0);
                        result.sqrDistance = sqrLength0;
                        result.closest[0] = point;
                        result.closest[1] = arc.end[0];
                        result.equidistant = false;
                    }
                    else
                    {
                        result.distance = std::sqrt(sqrLength1);
                        result.sqrDistance = sqrLength1;
                        result.closest[0] = point;
                        result.closest[1] = arc.end[1];
                        result.equidistant = false;
                    }
                }
            }
            else
            {
                // The point is the center of the circle containing the arc.
                result.distance = arc.radius;
                result.sqrDistance = arc.radius * arc.radius;
                result.closest[0] = point;
                result.closest[1] = arc.end[0];
                result.equidistant = true;
            }

            return result;
        }
    };
}
