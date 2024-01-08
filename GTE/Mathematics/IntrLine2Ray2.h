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
    class TIQuery<T, Line2<T>, Ray2<T>>
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

            // If the line and ray do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //
            // If the line and ray intersect in a single point,
            //   intersect = true
            //   numIntersections = 1
            //
            // If the line and ray are collinear,
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            bool intersect;
            int32_t numIntersections;
        };

        Result operator()(Line2<T> const& line, Ray2<T> const& ray)
        {
            Result result{};

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            auto llResult = llQuery(line, Line2<T>(ray.origin, ray.direction));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray.
                if (llResult.line1Parameter[0] >= static_cast<T>(0))
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
            else
            {
                result.intersect = llResult.intersect;
                result.numIntersections = llResult.numIntersections;
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Ray2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                lineParameter{ static_cast<T>(0), static_cast<T>(0) },
                rayParameter{ static_cast<T>(0), static_cast<T>(0) },
                point(Vector2<T>::Zero())
            {
            }

            // If the line and ray do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //   lineParameter[] = { 0, 0 }  // invalid
            //   rayParameter[] = { 0, 0 }  // invalid
            //   point = { 0, 0 }  // invalid
            //
            // If the line and ray intersect in a single point, the parameter
            // for line is s0 and the parameter for ray is s1 >= 0,
            //   intersect = true
            //   numIntersections = 1
            //   lineParameter = { s0, s0 }
            //   rayParameter = { s1, s1 }
            //   point = line.origin + s0 * line.direction
            //         = ray.origin + s1 * ray.direction
            //
            // If the line and ray are collinear, let
            // maxT = std::numeric_limits<T>::max(),
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            //   lineParameter[] = { -maxT, +maxT }
            //   rayParameter[] = { 0, +maxT }
            //   point = { 0, 0 }  // invalid
            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> lineParameter;
            std::array<T, 2> rayParameter;
            Vector2<T> point;
        };

        Result operator()(Line2<T> const& line, Ray2<T> const& ray)
        {
            Result result{};

            FIQuery<T, Line2<T>, Line2<T>> llQuery{};
            auto llResult = llQuery(line, Line2<T>(ray.origin, ray.direction));
            if (llResult.numIntersections == 1)
            {
                // Test whether the line-line intersection is on the ray.
                if (llResult.line1Parameter[0] >= static_cast<T>(0))
                {
                    result.intersect = true;
                    result.numIntersections = 1;
                    result.lineParameter[0] = llResult.line0Parameter[0];
                    result.lineParameter[1] = result.lineParameter[0];
                    result.rayParameter[0] = llResult.line1Parameter[0];
                    result.rayParameter[1] = result.rayParameter[0];
                    result.point = llResult.point;
                }
                else
                {
                    result.intersect = false;
                    result.numIntersections = 0;
                }
            }
            else if (llResult.numIntersections == std::numeric_limits<int32_t>::max())
            {
                result.intersect = true;
                result.numIntersections = std::numeric_limits<int32_t>::max();
                T maxT = std::numeric_limits<T>::max();
                result.lineParameter[0] = -maxT;
                result.lineParameter[1] = +maxT;
                result.rayParameter[0] = static_cast<T>(0);
                result.rayParameter[1] = +maxT;
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
