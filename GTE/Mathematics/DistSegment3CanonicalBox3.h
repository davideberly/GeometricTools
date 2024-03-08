// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a segment and a solid canonical box in 3D.
// 
// The segment is P0 + t * (P1 - P0) for 0 <= t <= 1. The direction D = P1-P0
// is generally not unit length.
// 
// The canonical box has center at the origin and is aligned with the
// coordinate axes. The extents are E = (e[0],e[1],e[2]). A box point is
// Y = (y[0],y[1],y[2]) with |y[i]| <= e[i] for all i.
// 
// The closest point on the segment is stored in closest[0] with parameter t.
// The closest point on the box is stored in closest[1]. When there are
// infinitely many choices for the pair of closest points, only one of them is
// returned.

#include <Mathematics/DistLine3CanonicalBox3.h>
#include <Mathematics/DistPointCanonicalBox.h>
#include <Mathematics/Segment.h>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Segment3<T>, CanonicalBox3<T>>
    {
    public:
        using LBQuery = DCPQuery<T, Line3<T>, CanonicalBox3<T>>;
        using Result = typename LBQuery::Result;

        Result operator()(Segment3<T> const& segment, CanonicalBox3<T> const& box)
        {
            Result result{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            Vector3<T> segDirection = segment.p[1] - segment.p[0];
            Line3<T> line(segment.p[0], segDirection);
            LBQuery lbQuery{};
            auto lbOutput = lbQuery(line, box);
            if (lbOutput.parameter >= zero)
            {
                if (lbOutput.parameter <= one)
                {
                    result = lbOutput;
                }
                else
                {
                    DCPQuery<T, Vector3<T>, CanonicalBox3<T>> pbQuery{};
                    auto pbOutput = pbQuery(segment.p[1], box);
                    result.sqrDistance = pbOutput.sqrDistance;
                    result.distance = pbOutput.distance;
                    result.parameter = one;
                    result.closest[0] = segment.p[1];
                    result.closest[1] = pbOutput.closest[1];
                }
            }
            else
            {
                DCPQuery<T, Vector3<T>, CanonicalBox3<T>> pbQuery{};
                auto pbOutput = pbQuery(segment.p[0], box);
                result.sqrDistance = pbOutput.sqrDistance;
                result.distance = pbOutput.distance;
                result.parameter = zero;
                result.closest[0] = segment.p[0];
                result.closest[1] = pbOutput.closest[1];
            }
            return result;
        }
    };
}
