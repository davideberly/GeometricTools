// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the arc to be a 1-dimensional object.

#include <Mathematics/IntrRay2Circle2.h>
#include <Mathematics/Arc2.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, Arc2<T>>
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

        Result operator()(Ray2<T> const& ray, Arc2<T> const& arc)
        {
            Result result{};
            FIQuery<T, Ray2<T>, Arc2<T>> raQuery{};
            auto raResult = raQuery(ray, arc);
            result.intersect = raResult.intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Ray2<T>, Arc2<T>>
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

        Result operator()(Ray2<T> const& ray, Arc2<T> const& arc)
        {
            Result result{};

            FIQuery<T, Ray2<T>, Circle2<T>> rcQuery{};
            Circle2<T> circle(arc.center, arc.radius);
            auto rcResult = rcQuery(ray, circle);
            if (rcResult.intersect)
            {
                // Test whether ray-circle intersections are on the arc.
                result.numIntersections = 0;
                for (int32_t i = 0; i < rcResult.numIntersections; ++i)
                {
                    if (arc.Contains(rcResult.point[i]))
                    {
                        result.intersect = true;
                        result.parameter[result.numIntersections] = rcResult.parameter[i];
                        result.point[result.numIntersections] = rcResult.point[i];
                        ++result.numIntersections;
                    }
                }
            }
            else
            {
                result.intersect = false;
                result.numIntersections = 0;
            }

            return result;
        }
    };
}
