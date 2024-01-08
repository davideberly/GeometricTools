// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.12.02

#pragma once

// Compute the intersection between a line and a solid rectangle in 3D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The intersection point, if any, is stored in result.point. The
// corresponding line parameter t is stored in result.parameter. The
// corresponding rectangle parameters s[] are stored in result.rectCoord[].
// When the line is in the plane of the rectangle and intersects the
// rectangle, the queries state that there are no intersections.
//
// TODO: Modify to support non-unit-length W[]. Return the point or
// segment of intersection when the line is in the plane of the rectangle
// and intersects the rectangle.

#include <Mathematics/FIQuery.h>
#include <Mathematics/TIQuery.h>
#include <Mathematics/Line.h>
#include <Mathematics/Rectangle.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class TIQuery<T, Line3<T>, Rectangle3<T>>
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

        Result operator()(Line3<T> const& line, Rectangle3<T> const& rectangle)
        {
            Result result{};

            // Compute the offset origin and rectangle normal.
            Vector3<T> diff = line.origin - rectangle.center;
            Vector3<T> normal = Cross(rectangle.axis[0], rectangle.axis[1]);

            // Solve Q + t*D = s0*W0 + s1*W1 (Q = diff, D = line direction,
            // W0 = edge 0 direction, W1 = edge 1 direction, N = Cross(W0,W1))
            // by
            //   s0 = Dot(W1,Cross(D,Q)) / Dot(D,N)
            //   s1 = -Dot(W0,Cross(D,Q)) / Dot(D,N)
            //   t = -Dot(Q,N) / Dot(D,N)
            T DdN = Dot(line.direction, normal);
            if (DdN == static_cast<T>(0))
            {
                // Line and triangle are parallel, call it a "no intersection"
                // even if the line and triangle are coplanar and
                // intersecting.
                result.intersect = false;
                return result;
            }

            T absDdN = std::fabs(DdN);
            Vector3<T> DxQ = Cross(line.direction, diff);
            T W1dDxQ = Dot(rectangle.axis[1], DxQ);
            if (std::fabs(W1dDxQ) <= rectangle.extent[0] * absDdN)
            {
                T W0dDxQ = Dot(rectangle.axis[0], DxQ);
                if (std::fabs(W0dDxQ) <= rectangle.extent[1] * absDdN)
                {
                    result.intersect = true;
                    return result;
                }
            }
            result.intersect = false;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Line3<T>, Rectangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                intersect(false),
                parameter(static_cast<T>(0)),
                rectCoord{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                point{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) }
            {
            }

            bool intersect;
            T parameter;
            std::array<T, 3> rectCoord;
            Vector3<T> point;
        };

        Result operator()(Line3<T> const& line, Rectangle3<T> const& rectangle)
        {
            Result result{};

            // Compute the offset origin and rectangle normal.
            Vector3<T> diff = line.origin - rectangle.center;
            Vector3<T> normal = Cross(rectangle.axis[0], rectangle.axis[1]);

            // Solve Q + t*D = s0*W0 + s1*W1 (Q = diff, D = line direction,
            // W0 = edge 0 direction, W1 = edge 1 direction, N = Cross(W0,W1))
            // by
            //   s0 = Dot(W1,Cross(D,Q)) / Dot(D,N)
            //   s1 = -Dot(W0,Cross(D,Q)) / Dot(D,N)
            //   t = -Dot(Q,N) / Dot(D,N)
            T DdN = Dot(line.direction, normal);
            if (DdN == static_cast<T>(0))
            {
                // Line and triangle are parallel, call it a "no intersection"
                // even if the line and triangle are coplanar and
                // intersecting.
                result.intersect = false;
                return result;
            }

            T absDdN = std::fabs(DdN);
            Vector3<T> DxQ = Cross(line.direction, diff);
            T W1dDxQ = Dot(rectangle.axis[1], DxQ);
            if (std::fabs(W1dDxQ) <= rectangle.extent[0] * absDdN)
            {
                T W0dDxQ = Dot(rectangle.axis[0], DxQ);
                if (std::fabs(W0dDxQ) <= rectangle.extent[1] * absDdN)
                {
                    result.intersect = true;
                    result.parameter = -Dot(diff, normal) / DdN;
                    result.rectCoord[0] = W1dDxQ / DdN;
                    result.rectCoord[1] = -W0dDxQ / DdN;
                    result.point = line.origin + result.parameter * line.direction;
                    return result;
                }
            }
            result.intersect = false;
            return result;
        }
    };
}
