// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

#include <Mathematics/IntrLine2Line2.h>
#include <Mathematics/Ray.h>
#include <array>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Ray2<T>, Ray2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0)
            {
            }

            // The number is 0 (no intersection), 1 (rays intersect in a
            // single point), 2 (rays are collinear and intersect in a 
            // segment; ray directions are opposite of each other), or
            // std::numeric_limits<int32_t>::max() (intersection is a ray; ray
            // directions are the same).
            bool intersect;
            int32_t numIntersections;
        };

        Result operator()(Ray2<T> const& ray0, Ray2<T> const& ray1)
        {
            Result result{};

            T const zero = static_cast<T>(0);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(ray0.origin, ray0.direction);
            Line2<T> line1(ray1.origin, ray1.direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the rays.
                if (llResult.line0Parameter[0] >= zero &&
                    llResult.line1Parameter[0] >= zero)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                if (Dot(ray0.direction, ray1.direction) > zero)
                {
                    // The rays are collinear and in the same direction, so
                    // they must overlap.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int32_t>::max();
                }
                else
                {
                    // The rays are collinear but in opposite directions.
                    // Test whether they overlap.  Ray0 has interval
                    // [0,+infinity) and ray1 has interval (-infinity,t]
                    // relative to ray0.direction.
                    Vector2<T> diff = ray1.origin - ray0.origin;
                    T t = Dot(ray0.direction, diff);
                    if (t > zero)
                    {
                        result.intersect = true;
                        result.numIntersections = 2;
                    }
                    else if (t < zero)
                    {
                        result.intersect = false;
                        result.numIntersections = 0;
                    }
                    else  // t == 0
                    {
                        result.intersect = true;
                        result.numIntersections = 1;
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

    template <typename T>
    class FIQuery<T, Ray2<T>, Ray2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                ray0Parameter{ static_cast<T>(0), static_cast<T>(0) },
                ray1Parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{ Vector2<T>::Zero(), Vector2<T>::Zero() }
            {
            }

            // The number is 0 (no intersection), 1 (rays intersect in a
            // single point), 2 (rays are collinear and intersect in a 
            // segment; ray directions are opposite of each other), or
            // std::numeric_limits<int32_t>::max() (intersection is a ray; ray
            // directions are the same).
            bool intersect;
            int32_t numIntersections;

            // If numIntersections is 1, the intersection is
            //   point[0] = ray0.origin + ray0Parameter[0] * ray0.direction
            //            = ray1.origin + ray1Parameter[0] * ray1.direction
            // If numIntersections is 2, the segment of intersection is formed
            // by the ray origins,
            //   ray0Parameter[0] = ray1Parameter[0] = 0
            //   point[0] = ray0.origin
            //            = ray1.origin + ray1Parameter[1] * ray1.direction
            //   point[1] = ray1.origin
            //            = ray0.origin + ray0Parameter[1] * ray0.direction
            // where ray0Parameter[1] >= 0 and ray1Parameter[1] >= 0.
            // If numIntersections is maxInt, let
            //   ray1.origin = ray0.origin + t * ray0.direction
            // then
            //   ray0Parameter[] = { max(t,0), +maxReal }
            //   ray1Parameter[] = { -min(t,0), +maxReal }
            //   point[0] = ray0.origin + ray0Parameter[0] * ray0.direction
            std::array<T, 2> ray0Parameter, ray1Parameter;
            std::array<Vector2<T>, 2> point;
        };

        Result operator()(Ray2<T> const& ray0, Ray2<T> const& ray1)
        {
            Result result{};

            T const zero = static_cast<T>(0);

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            Line2<T> line0(ray0.origin, ray0.direction);
            Line2<T> line1(ray1.origin, ray1.direction);
            auto llResult = llQuery(line0, line1);
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the rays.
                if (llResult.line0Parameter[0] >= zero &&
                    llResult.line1Parameter[0] >= zero)
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.ray0Parameter[0] = llResult.line0Parameter[0];
                    result.ray1Parameter[0] = llResult.line1Parameter[0];
                    result.point[0] = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                // Compute t for which ray1.origin =
                // ray0.origin + t*ray0.direction.
                T maxReal = std::numeric_limits<T>::max();
                Vector2<T> diff = ray1.origin - ray0.origin;
                T t = Dot(ray0.direction, diff);
                if (Dot(ray0.direction, ray1.direction) > zero)
                {
                    // The rays are collinear and in the same direction, so
                    // they must overlap.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int32_t>::max();
                    if (t >= zero)
                    {
                        result.ray0Parameter[0] = t;
                        result.ray0Parameter[1] = maxReal;
                        result.ray1Parameter[0] = zero;
                        result.ray1Parameter[1] = maxReal;
                        result.point[0] = ray1.origin;
                    }
                    else
                    {
                        result.ray0Parameter[0] = zero;
                        result.ray0Parameter[1] = maxReal;
                        result.ray1Parameter[0] = -t;
                        result.ray1Parameter[1] = maxReal;
                        result.point[0] = ray0.origin;
                    }
                }
                else
                {
                    // The rays are collinear but in opposite directions.
                    // Test whether they overlap. Ray0 has interval
                    // [0,+infinity) and ray1 has interval (-infinity,t1]
                    // relative to ray0.direction.
                    if (t >= zero)
                    {
                        result.intersect = true;
                        result.numIntersections = 2;
                        result.ray0Parameter[0] = zero;
                        result.ray0Parameter[1] = t;
                        result.ray1Parameter[0] = zero;
                        result.ray1Parameter[1] = t;
                        result.point[0] = ray0.origin;
                        result.point[1] = ray1.origin;
                    }
                    else
                    {
                        result.intersect = false;
                        result.numIntersections = 0;
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
