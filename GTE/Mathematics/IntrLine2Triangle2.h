// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The queries consider the triangle to be a solid. The algorithms are based
// on determining on which side of the line the vertices lie. The test uses
// the sign of the projections of the vertices onto a normal line that is
// perpendicular to the specified line. The table of possibilities is listed
// next with n = numNegative, p = numPositive and z = numZero.
//
//   n p z  intersection
//   ------------------------------------
//   0 3 0  none
//   0 2 1  vertex
//   0 1 2  edge
//   0 0 3  none (degenerate triangle)
//   1 2 0  segment (2 edges clipped)
//   1 1 1  segment (1 edge clipped)
//   1 0 2  edge
//   2 1 0  segment (2 edges clipped)
//   2 0 1  vertex
//   3 0 0  none
//
// The case (n,p,z) = (0,0,3) is treated as a no-intersection because the
// triangle is degenerate.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Triangle.h>
#include <Mathematics/Vector2.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line2<T>, Triangle2<T>>
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

        // The line is P + t * D, where P is a point on the line and D is a
        // direction vector that does not have to be unit length. This is
        // useful when using a 2-point representation P0 + t * (P1 - P0).
        Result operator()(Line2<T> const& line, Triangle2<T> const& triangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            int32_t numPositive = 0, numNegative = 0, numZero = 0;
            for (size_t i = 0; i < 3; ++i)
            {
                Vector2<T> diff = triangle.v[i] - line.origin;
                T s = DotPerp(line.direction, diff);
                if (s > zero)
                {
                    ++numPositive;
                }
                else if (s < zero)
                {
                    ++numNegative;
                }
                else
                {
                    ++numZero;
                }
            }

            result.intersect =
                (numZero == 0 && numPositive > 0 && numNegative > 0) ||
                (numZero == 1) ||
                (numZero == 2);

            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line2<T>, Triangle2<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                numIntersections(0),
                parameter{ static_cast<T>(0), static_cast<T>(0) },
                point{
                    Vector2<T>{ static_cast<T>(0), static_cast<T>(0) },
                    Vector2<T>{ static_cast<T>(0), static_cast<T>(0) }}
            {
            }

            bool intersect;
            int32_t numIntersections;
            std::array<T, 2> parameter;
            std::array<Vector2<T>, 2> point;
        };

        // The line is P + t * D, where P is a point on the line and D is a
        // direction vector that does not have to be unit length. This is
        // useful when using a 2-point representation P0 + t * (P1 - P0).
        Result operator()(Line2<T> const& line, Triangle2<T> const& triangle)
        {
            Result result{};
            DoQuery(line.origin, line.direction, triangle, result);
            if (result.intersect)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    result.point[i] = line.origin + result.parameter[i] * line.direction;
                }
            }
            return result;
        }

    protected:
        // The caller must ensure that on entry, 'result' is default
        // constructed as if there is no intersection. If an intersection is
        // found, the 'result' values will be modified accordingly.
        void DoQuery(Vector2<T> const& origin, Vector2<T> const& direction,
            Triangle2<T> const& triangle, Result& result)
        {
            T const zero = static_cast<T>(0);
            std::array<T, 3> s{ zero, zero, zero };
            int32_t numPositive = 0, numNegative = 0, numZero = 0;
            for (size_t i = 0; i < 3; ++i)
            {
                Vector2<T> diff = triangle.v[i] - origin;
                s[i] = DotPerp(direction, diff);
                if (s[i] > zero)
                {
                    ++numPositive;
                }
                else if (s[i] < zero)
                {
                    ++numNegative;
                }
                else
                {
                    ++numZero;
                }
            }

            if (numZero == 0 && numPositive > 0 && numNegative > 0)
            {
                // (n,p,z) is (1,2,0) or (2,1,0).
                result.intersect = true;
                result.numIntersections = 2;

                // sign is +1 when (n,p) is (2,1) or -1 when (n,p) is (1,2).
                T sign = (3 > numPositive * 2 ? static_cast<T>(1) : static_cast<T>(-1));
                for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                {
                    if (sign * s[i2] > zero)
                    {
                        Vector2<T> diffVi0P0 = triangle.v[i0] - origin;
                        Vector2<T> diffVi2Vi0 = triangle.v[i2] - triangle.v[i0];
                        T lambda0 = s[i0] / (s[i0] - s[i2]);
                        Vector2<T> q0 = diffVi0P0 + lambda0 * diffVi2Vi0;
                        result.parameter[0] = Dot(direction, q0);
                        Vector2<T> diffVi1P0 = triangle.v[i1] - origin;
                        Vector2<T> diffVi2Vi1 = triangle.v[i2] - triangle.v[i1];
                        T lambda1 = s[i1] / (s[i1] - s[i2]);
                        Vector2<T> q1 = diffVi1P0 + lambda1 * diffVi2Vi1;
                        result.parameter[1] = Dot(direction, q1);
                        break;
                    }
                }
            }
            else if (numZero == 1)
            {
                // (n,p,z) is (1,1,1), (2,0,1) or (0,2,1).
                result.intersect = true;
                for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                {
                    if (s[i2] == zero)
                    {
                        Vector2<T> diffVi2P0 = triangle.v[i2] - origin;
                        result.parameter[0] = Dot(direction, diffVi2P0);
                        if (numPositive == 2 || numNegative == 2)
                        {
                            // (n,p,z) is (2,0,1) or (0,2,1).
                            result.numIntersections = 1;
                            result.parameter[1] = result.parameter[0];
                        }
                        else
                        {
                            // (n,p,z) is (1,1,1).
                            result.numIntersections = 2;
                            Vector2<T> diffVi0P0 = triangle.v[i0] - origin;
                            Vector2<T> diffVi1Vi0 = triangle.v[i1] - triangle.v[i0];
                            T lambda0 = s[i0] / (s[i0] - s[i1]);
                            Vector2<T> q = diffVi0P0 + lambda0 * diffVi1Vi0;
                            result.parameter[1] = Dot(direction, q);
                        }
                        break;
                    }
                }
            }
            else if (numZero == 2)
            {
                // (n,p,z) is (1,0,2) or (0,1,2).
                result.intersect = true;
                result.numIntersections = 2;
                for (size_t i0 = 1, i1 = 2, i2 = 0; i2 < 3; i0 = i1, i1 = i2++)
                {
                    if (s[i2] != zero)
                    {
                        Vector2<T> diffVi0P0 = triangle.v[i0] - origin;
                        result.parameter[0] = Dot(direction, diffVi0P0);
                        Vector2<T> diffVi1P0 = triangle.v[i1] - origin;
                        result.parameter[1] = Dot(direction, diffVi1P0);
                        break;
                    }
                }
            }
            // else: (n,p,z) is (3,0,0), (0,3,0) or (0,0,3). The constructor
            // for Result initializes all members to zero, so no additional
            // assignments are needed for 'result'.

            if (result.intersect)
            {
                T directionSqrLength = Dot(direction, direction);
                result.parameter[0] /= directionSqrLength;
                result.parameter[1] /= directionSqrLength;
                if (result.parameter[0] > result.parameter[1])
                {
                    std::swap(result.parameter[0], result.parameter[1]);
                }
            }
        }
    };
}
