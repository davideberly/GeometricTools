// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Test-intersection and find-intersection queries for two lines. The line
// directions are nonzero but not required to be unit length.

#include <Mathematics/Vector2.h>
#include <Mathematics/Line.h>
#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, Line2<T>>
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

            // If the lines do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //
            // If the lines intersect in a single point,
            //   intersect = true
            //   numIntersections = 1
            //
            // If the lines are the same,
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            bool intersect;
            int32_t numIntersections;
        };

        Result operator()(Line2<T> const& line0, Line2<T> const& line1)
        {
            Result result{};

            // The intersection of two lines is a solution to P0 + s0 * D0 =
            // P1 + s1 * D1. Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q. If
            // DotPerp(D0, D1)) = 0, the lines are parallel. Additionally, if
            // DotPerp(Q, D1)) = 0, the lines are the same. If
            // DotPerp(D0, D1)) is not zero, then the lines intersect in a
            // single point where
            //   s0 = DotPerp(Q, D1))/DotPerp(D0, D1))
            //   s1 = DotPerp(Q, D0))/DotPerp(D0, D1))

            T const zero = static_cast<T>(0);
            T dotD0PerpD1 = DotPerp(line0.direction, line1.direction);
            if (dotD0PerpD1 != zero)
            {
                // The lines are not parallel.
                result.intersect = true;
                result.numIntersections = 1;
            }
            else
            {
                // The lines are parallel.
                Vector2<T> Q = line1.origin - line0.origin;
                T dotQDotPerpD1 = DotPerp(Q, line1.direction);
                if (dotQDotPerpD1 != zero)
                {
                    // The lines are parallel but distinct.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
                else
                {
                    // The lines are the same.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int32_t>::max();
                }
            }

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Line2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                line0Parameter{ static_cast<T>(0), static_cast<T>(0) },
                line1Parameter{ static_cast<T>(0), static_cast<T>(0) },
                point(Vector2<T>::Zero())
            {
            }

            // If the lines do not intersect,
            //   intersect = false
            //   numIntersections = 0
            //   line0Parameter[] = { 0, 0 }  // invalid
            //   line1Parameter[] = { 0, 0 }  // invalid
            //   point = { 0, 0 }  // invalid
            //
            // If the lines intersect in a single point, the parameter for
            // line0 is s0 and the parameter for line1 is s1,
            //   intersect = true
            //   numIntersections = 1
            //   line0Parameter = { s0, s0 }
            //   line1Parameter = { s1, s1 }
            //   point = line0.origin + s0 * line0.direction
            //         = line1.origin + s1 * line1.direction
            //
            // If the lines are the same,
            // let maxT = std::numeric_limits<T>::max(),
            //   intersect = true
            //   numIntersections = std::numeric_limits<int32_t>::max()
            //   line0Parameter = { -maxT, +maxT }
            //   line1Parameter = { -maxT, +maxT }
            //   point = { 0, 0 }  // invalid
            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> line0Parameter, line1Parameter;
            Vector2<T> point;
        };

        Result operator()(Line2<T> const& line0, Line2<T> const& line1)
        {
            Result result{};

            // The intersection of two lines is a solution to P0 + s0 * D0 =
            // P1 + s1 * D1. Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q. If
            // DotPerp(D0, D1)) = 0, the lines are parallel. Additionally, if
            // DotPerp(Q, D1)) = 0, the lines are the same. If
            // DotPerp(D0, D1)) is not zero, then the lines intersect in a
            // single point where
            //   s0 = DotPerp(Q, D1))/DotPerp(D0, D1))
            //   s1 = DotPerp(Q, D0))/DotPerp(D0, D1))

            T const zero = static_cast<T>(0);
            Vector2<T> Q = line1.origin - line0.origin;
            T dotD0PerpD1 = DotPerp(line0.direction, line1.direction);
            if (dotD0PerpD1 != zero)
            {
                // The lines are not parallel.
                result.intersect = true;
                result.numIntersections = 1;
                T dotQPerpD0 = DotPerp(Q, line0.direction);
                T dotQPerpD1 = DotPerp(Q, line1.direction);
                T s0 = dotQPerpD1 / dotD0PerpD1;
                T s1 = dotQPerpD0 / dotD0PerpD1;
                result.line0Parameter = { s0, s0 };
                result.line1Parameter = { s1, s1 };
                result.point = line0.origin + s0 * line0.direction;
            }
            else
            {
                // The lines are parallel.
                T dotQPerpD1 = DotPerp(Q, line1.direction);
                if (std::fabs(dotQPerpD1) != zero)
                {
                    // The lines are parallel but distinct.
                    result.intersect = false;
                    result.numIntersections = 0;
                }
                else
                {
                    // The lines are the same.
                    result.intersect = true;
                    result.numIntersections = std::numeric_limits<int32_t>::max();
                    T const maxT = std::numeric_limits<T>::max();
                    result.line0Parameter = { -maxT, maxT };
                    result.line1Parameter = { -maxT, maxT };
                }
            }

            return result;
        }
    };
}
