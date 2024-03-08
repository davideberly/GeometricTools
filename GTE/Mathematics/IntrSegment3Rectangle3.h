// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.9.2023.12.02

#pragma once

// Compute the intersection between a segment and a solid rectangle in 3D.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The intersection point, if any, is stored in result.point. The
// corresponding segment parameter t is stored in result.parameter. The
// corresponding rectangle parameters s[] are stored in result.rectCoord[].
// When the segment is in the plane of the rectangle and intersects the
// rectangle, the queries state that there are no intersections.
//
// TODO: Modify to support non-unit-length W[]. Return the point or
// segment of intersection when the segment is in the plane of the rectangle
// and intersects the rectangle.

#include <Mathematics/IntrLine3Rectangle3.h>
#include <Mathematics/Segment.h>
#include <array>

namespace gte
{
    template <typename T>
    class TIQuery<T, Segment3<T>, Rectangle3<T>>
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

        Result operator()(Segment3<T> const& segment, Rectangle3<T> const& rectangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            FIQuery<T, Line3<T>, Rectangle3<T>> lrQuery{};
            Line3<T> line(segment.p[0], segment.p[1] - segment.p[0]);
            auto lrResult = lrQuery(line, rectangle);
            if (lrResult.intersect)
            {
                if (zero <= lrResult.parameter && lrResult.parameter <= one)
                {
                    // The line-rectangle intersection is on the segment.
                    result.intersect = true;
                    return result;
                }
            }

            result.intersect = false;
            return result;
        }
    };

    template <typename T>
    class FIQuery<T, Segment3<T>, Rectangle3<T>>
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

        Result operator()(Segment3<T> const& segment, Rectangle3<T> const& rectangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            FIQuery<T, Line3<T>, Rectangle3<T>> lrQuery{};
            Line3<T> line(segment.p[0], segment.p[1] - segment.p[0]);
            auto lrResult = lrQuery(line, rectangle);
            if (lrResult.intersect)
            {
                if (zero <= lrResult.parameter && lrResult.parameter <= one)
                {
                    // The line-rectangle intersection is on the segment.
                    result.intersect = true;
                    result.parameter = lrResult.parameter;
                    result.rectCoord = lrResult.rectCoord;
                    result.point = lrResult.point;
                    return result;
                }
            }

            result.intersect = false;
            return result;
        }
    };
}
