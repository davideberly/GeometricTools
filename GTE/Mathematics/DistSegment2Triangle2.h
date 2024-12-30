// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.3.2024.12.29

#pragma once

// Compute the distance between a segment and a solid triangle in 3D.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on the segment is stored in closest[0] with parameter t.
// The closest point on the triangle is closest[1] with barycentric
// coordinates (b[0],b[1],b[2]). When there are infinitely many choices for the pair of closest
// points, only one of them is returned.

#include <Mathematics/DistLine2Triangle2.h>
#include <Mathematics/DistPointTriangle.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment2<T>, Triangle2<T>>
    {
    public:
        using LTQuery = DCPQuery<T, Line2<T>, Triangle2<T>>;
        using Result = typename LTQuery::Result;

        Result operator()(Segment2<T> const& segment, Triangle2<T> const& triangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector2<T> segDirection = segment.p[1] - segment.p[0];
            Line2<T> line(segment.p[0], segDirection);
            LTQuery ltQuery{};
            auto ltResult = ltQuery(line, triangle);
            if (ltResult.parameter >= zero)
            {
                if (ltResult.parameter <= one)
                {
                    result = ltResult;
                }
                else
                {
                    DCPQuery<T, Vector2<T>, Triangle2<T>> ptQuery{};
                    auto ptResult = ptQuery(segment.p[1], triangle);
                    result.distance = ptResult.distance;
                    result.sqrDistance = ptResult.sqrDistance;
                    result.parameter = one;
                    result.barycentric = ptResult.barycentric;
                    result.closest[0] = segment.p[1];
                    result.closest[1] = ptResult.closest[1];
                }
            }
            else
            {
                DCPQuery<T, Vector2<T>, Triangle2<T>> ptQuery{};
                auto ptResult = ptQuery(segment.p[0], triangle);
                result.distance = ptResult.distance;
                result.sqrDistance = ptResult.sqrDistance;
                result.parameter = zero;
                result.barycentric = ptResult.barycentric;
                result.closest[0] = segment.p[0];
                result.closest[1] = ptResult.closest[1];
            }
            return result;
        }
    };
}
