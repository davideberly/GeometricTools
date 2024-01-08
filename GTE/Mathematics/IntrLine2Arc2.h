// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the arc to be a 1-dimensional object.

#include <Mathematics/IntrLine2Circle2.h>
#include <Mathematics/Arc2.h>
#include <array>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, Arc2<T>>
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

        Result operator()(Line2<T> const& line, Arc2<T> const& arc)
        {
            Result result{};
            FIQuery<T, Line2<T>, Arc2<T>> laQuery;
            auto laResult = laQuery(line, arc);
            result.intersect = laResult.intersect;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Arc2<T>>
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

        Result operator()(Line2<T> const& line, Arc2<T> const& arc)
        {
            Result result{};

            FIQuery<T, Line2<T>, Circle2<T>> lcQuery;
            Circle2<T> circle(arc.center, arc.radius);
            auto lcResult = lcQuery(line, circle);
            if (lcResult.intersect)
            {
                // Test whether line-circle intersections are on the arc.
                result.numIntersections = 0;
                for (int32_t i = 0; i < lcResult.numIntersections; ++i)
                {
                    if (arc.Contains(lcResult.point[i]))
                    {
                        result.intersect = true;
                        result.parameter[result.numIntersections] = lcResult.parameter[i];
                        result.point[result.numIntersections] = lcResult.point[i];
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
