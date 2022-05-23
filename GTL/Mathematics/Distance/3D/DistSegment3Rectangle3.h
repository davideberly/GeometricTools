// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistLine3Rectangle3.h>
#include <GTL/Mathematics/Distance/ND/DistPointRectangle.h>
#include <GTL/Mathematics/Primitives/ND/Segment.h>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Segment3<T>, Rectangle3<T>>
    {
    public:
        using LRQuery = DCPQuery<T, Line3<T>, Rectangle3<T>>;
        using Output = typename LRQuery::Output;

        Output operator()(Segment3<T> const& segment, Rectangle3<T> const& rectangle)
        {
            Output output{};

            Vector3<T> segDirection = segment.p[1] - segment.p[0];
            Line3<T> line(segment.p[0], segDirection);
            LRQuery lrQuery{};
            auto lrResult = lrQuery(line, rectangle);
            if (lrResult.parameter >= C_<T>(0))
            {
                if (lrResult.parameter <= C_<T>(1))
                {
                    output = lrResult;
                }
                else
                {
                    DCPQuery<T, Vector3<T>, Rectangle3<T>> prQuery{};
                    auto prOutput = prQuery(segment.p[1], rectangle);
                    output.distance = prOutput.distance;
                    output.sqrDistance = prOutput.sqrDistance;
                    output.parameter = C_<T>(1);
                    output.cartesian = prOutput.cartesian;
                    output.closest[0] = segment.p[1];
                    output.closest[1] = prOutput.closest[1];
                }
            }
            else
            {
                DCPQuery<T, Vector3<T>, Rectangle3<T>> prQuery{};
                auto prOutput = prQuery(segment.p[0], rectangle);
                output.distance = prOutput.distance;
                output.sqrDistance = prOutput.sqrDistance;
                output.parameter = C_<T>(0);
                output.cartesian = prOutput.cartesian;
                output.closest[0] = segment.p[0];
                output.closest[1] = prOutput.closest[1];
            }
            return output;
        }
    };
}
