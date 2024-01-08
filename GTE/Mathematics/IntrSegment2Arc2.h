// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the arc to be a 1-dimensional object.

#include <Mathematics/IntrSegment2Circle2.h>
#include <Mathematics/Arc2.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment2<T>, Arc2<T>>
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

        Result operator()(Segment2<T> const& segment, Arc2<T> const& arc)
        {
            Result result{};
            FIQuery<T, Segment2<T>, Arc2<T>> saQuery{};
            auto saResult = saQuery(segment, arc);
            result.intersect = saResult.intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment2<T>, Arc2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ (T)0, (T)0 },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Segment2<T> const& segment, Arc2<T> const& arc)
        {
            Result result{};
            result.intersect = false;
            result.numIntersections = 0;
            result.parameter[0] = (T)0;
            result.parameter[0] = (T)0;
            result.point[0] = { (T)0, (T)0 };
            result.point[1] = { (T)0, (T)0 };

            FIQuery<T, Segment2<T>, Circle2<T>> scQuery{};
            Circle2<T> circle(arc.center, arc.radius);
            auto scResult = scQuery(segment, circle);
            if (scResult.intersect)
            {
                // Test whether line-circle intersections are on the arc.
                for (int32_t i = 0; i < scResult.numIntersections; ++i)
                {
                    if (arc.Contains(scResult.point[i]))
                    {
                        result.intersect = true;
                        result.parameter[result.numIntersections] = scResult.parameter[i];
                        result.point[result.numIntersections] = scResult.point[i];
                        ++result.numIntersections;
                    }
                }
            }

            return result;
        }
    };
}
