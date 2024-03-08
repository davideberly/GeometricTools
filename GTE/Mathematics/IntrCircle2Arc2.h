// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrCircle2Circle2.h>
#include <Mathematics/Arc2.h>
#include <array>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class FIQuery<T, Circle2<T>, Arc2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() },
                arc(Vector2<T>::Zero(), (T)0, Vector2<T>::Zero(), Vector2<T>::Zero())
            {
            }

            bool intersect;

            // The number of intersections is 0, 1, 2 or maxInt =
            // std::numeric_limits<int32_t>::max().  When 1, the arc and circle
            // intersect in a single point.  When 2, the arc is not on the
            // circle and they intersect in two points.  When maxInt, the
            // arc is on the circle.
            int32_t numIntersections;

            // Valid only when numIntersections = 1 or 2.
            std::array<Vector2<T>, 2> point;

            // Valid only when numIntersections = maxInt.
            Arc2<T> arc;
        };

        Result operator()(Circle2<T> const& circle, Arc2<T> const& arc)
        {
            Result result{};

            Circle2<T> circleOfArc(arc.center, arc.radius);
            FIQuery<T, Circle2<T>, Circle2<T>> ccQuery;
            auto ccResult = ccQuery(circle, circleOfArc);
            if (!ccResult.intersect)
            {
                result.intersect = false;
                result.numIntersections = 0;
                return result;
            }

            if (ccResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                // The arc is on the circle.
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int32_t>::max();
                result.arc = arc;
                return result;
            }

            // Test whether circle-circle intersection points are on the arc.
            for (int32_t i = 0; i < ccResult.numIntersections; ++i)
            {
                result.numIntersections = 0;
                if (arc.Contains(ccResult.point[i]))
                {
                    result.point[result.numIntersections++] = ccResult.point[i];
                    result.intersect = true;
                }
            }
            return result;
        }
    };
}
