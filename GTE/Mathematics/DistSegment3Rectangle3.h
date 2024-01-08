// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a segment and a solid rectangle in 3D.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The closest point on the segment is stored in closest[0] with parameter t.
// The closest point on the rectangle is closest[1] with W-coordinates
// (s[0],s[1]). When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <Mathematics/DistLine3Rectangle3.h>
#include <Mathematics/DistPointRectangle.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment3<T>, Rectangle3<T>>
    {
    public:
        using LRQuery = DCPQuery<T, Line3<T>, Rectangle3<T>>;
        using Result = typename LRQuery::Result;

        Result operator()(Segment3<T> const& segment, Rectangle3<T> const& rectangle)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector3<T> segDirection = segment.p[1] - segment.p[0];
            Line3<T> line(segment.p[0], segDirection);
            LRQuery lrQuery{};
            auto lrResult = lrQuery(line, rectangle);
            if (lrResult.parameter >= zero)
            {
                if (lrResult.parameter <= one)
                {
                    result = lrResult;
                }
                else
                {
                    DCPQuery<T, Vector3<T>, Rectangle3<T>> prQuery{};
                    auto prResult = prQuery(segment.p[1], rectangle);
                    result.distance = prResult.distance;
                    result.sqrDistance = prResult.sqrDistance;
                    result.parameter = one;
                    result.cartesian = prResult.cartesian;
                    result.closest[0] = segment.p[1];
                    result.closest[1] = prResult.closest[1];
                }
            }
            else
            {
                DCPQuery<T, Vector3<T>, Rectangle3<T>> prQuery{};
                auto prResult = prQuery(segment.p[0], rectangle);
                result.distance = prResult.distance;
                result.sqrDistance = prResult.sqrDistance;
                result.parameter = zero;
                result.cartesian = prResult.cartesian;
                result.closest[0] = segment.p[0];
                result.closest[1] = prResult.closest[1];
            }
            return result;
        }
    };
}
