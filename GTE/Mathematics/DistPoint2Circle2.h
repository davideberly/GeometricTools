// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2023.08.08

#pragma once

#include <Mathematics/DCPQuery.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Vector2.h>
#include <array>
#include <cmath>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector2<T>, Circle2<T>>
    {
    public:
        // The input point is stored in the member closest[0]. If a single
        // point on the circle is closest to the input point, the member
        // closest[1] is set to that point and the equidistant member is set
        // to false. If the entire circle is equidistant to the point, the
        // member closest[1] is set to C+r*(1,0), where C is the circle
        // center and r is the circle radius. Moreover, the equidistant
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

        Result operator()(Vector2<T> const& point, Circle2<T> const& circle)
        {
            Result result{};

            Vector2<T> diff = point - circle.center;
            T sqrLength = Dot(diff, diff);
            T length = std::sqrt(sqrLength);
            if (length > static_cast<T>(0))
            {
                diff /= length;
                result.distance = std::fabs(length - circle.radius);
                result.sqrDistance = result.distance * result.distance;
                result.closest[0] = point;
                result.closest[1] = circle.center + circle.radius * diff;
                result.equidistant = false;
            }
            else
            {
                result.distance = circle.radius;
                result.sqrDistance = circle.radius * circle.radius;
                result.closest[0] = point;
                result.closest[1] = circle.center + circle.radius * Vector2<T>::Unit(0);
                result.equidistant = true;
            }

            return result;
        }
    };
}
