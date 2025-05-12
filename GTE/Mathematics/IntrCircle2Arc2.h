// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

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

#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
            // The code analysis tool complained:
            // warning C28020: The expression '0<=_Param_(1)&&_Param_(1)<=2-1'
            // is not true at this call.: Lines: 51, 53, 54, 55, 56, 63, 73,
            // 74, 76, 78, 79, 74, 76, 78. Before inserting this comment, line
            // 74 is the for-loop below. The code analysis tool seems to
            // believe that result.numIntersections is 2 and that this number
            // is out-of-range. It is not because ccResult sets the value of
            // numIntersections to 0, 1, 2 or maxInt. The test for maxInt
            // occurs in the if-statement above. Inferring that the number of
            // intersections at this point being 0, 1, or 2 is probably
            // difficult for a code analysis tool.
#endif
            // Test whether circle-circle intersection points are on the arc.
            result.numIntersections = 0;
            for (int32_t i = 0; i < ccResult.numIntersections; ++i)
            {
                if (arc.Contains(ccResult.point[i]))
                {
                    result.point[result.numIntersections++] = ccResult.point[i];
                    result.intersect = true;
                }
            }
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
            return result;
        }
    };
}

