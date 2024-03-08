// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a segment and a solid aligned box in 2D.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The aligned box has minimum corner A and maximum corner B. A box point is X
// where A <= X <= B; the comparisons are componentwise.
// 
// The closest point on the segment is stored in closest[0] with parameter t.
// The closest point on the box is stored in closest[1]. When there are
// infinitely many choices for the pair of closest points, only one of them is
// returned.

#include <Mathematics/DistLine2AlignedBox2.h>
#include <Mathematics/DistPointAlignedBox.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment2<T>, AlignedBox2<T>>
    {
    public:
        using AlignedQuery = DCPQuery<T, Line2<T>, AlignedBox2<T>>;
        using Result = typename AlignedQuery::Result;

        Result operator()(Segment2<T> const& segment, AlignedBox2<T> const& box)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector2<T> direction = segment.p[1] - segment.p[0];
            Line2<T> line(segment.p[0], direction);
            DCPQuery<T, Line2<T>, AlignedBox2<T>> lbQuery{};
            auto lbResult = lbQuery(line, box);
            if (lbResult.parameter >= zero)
            {
                if (lbResult.parameter <= one)
                {
                    result = lbResult;
                }
                else
                {
                    DCPQuery<T, Vector2<T>, AlignedBox2<T>> pbQuery{};
                    auto pbResult = pbQuery(segment.p[1], box);
                    result.sqrDistance = pbResult.sqrDistance;
                    result.distance = pbResult.distance;
                    result.parameter = one;
                    result.closest[0] = segment.p[1];
                    result.closest[1] = pbResult.closest[1];
                }
            }
            else
            {
                DCPQuery<T, Vector2<T>, AlignedBox2<T>> pbQuery{};
                auto pbResult = pbQuery(segment.p[0], box);
                result.distance = pbResult.distance;
                result.sqrDistance = pbResult.sqrDistance;
                result.parameter = zero;
                result.closest[0] = segment.p[0];
                result.closest[1] = pbResult.closest[1];
            }

            return result;
        }
    };
}
