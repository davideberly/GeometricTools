// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a line and a solid rectangle in 3D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the rectangle is stored in closest[1] with W-coordinates
// (s[0],s[1]). When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <GTL/Mathematics/Distance/ND/DistLineSegment.h>
#include <GTL/Mathematics/Primitives/ND/Rectangle.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Rectangle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter(C_<T>(0)),
                cartesian{ C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Line3<T> const& line, Rectangle3<T> const& rectangle)
        {
            Output output{};

            // Test whether the line intersects rectangle. If so, the squared
            // distance is zero. The normal of the plane of the rectangle does
            // not have to be normalized to unit length.
            Vector3<T> N = Cross(rectangle.axis[0], rectangle.axis[1]);
            T NdD = Dot(N, line.direction);
            if (std::fabs(NdD) > C_<T>(0))
            {
                // The line and rectangle are not parallel, so the line
                // intersects the plane of the rectangle at a point Y.
                // Determine whether Y is contained by the rectangle.
                Vector3<T> PmC = line.origin - rectangle.center;
                T NdDiff = Dot(N, PmC);
                T tIntersect = -NdDiff / NdD;
                Vector3<T> Y = line.origin + tIntersect * line.direction;
                Vector3<T> YmC = Y - rectangle.center;

                // Compute the rectangle coordinates of the intersection.
                T s0 = Dot(rectangle.axis[0], YmC);
                T s1 = Dot(rectangle.axis[1], YmC);

                if (std::fabs(s0) <= rectangle.extent[0] &&
                    std::fabs(s1) <= rectangle.extent[1])
                {
                    // The point Y is contained by the rectangle.
                    output.distance = C_<T>(0);
                    output.sqrDistance = C_<T>(0);
                    output.parameter = tIntersect;
                    output.cartesian[0] = s0;
                    output.cartesian[1] = s1;
                    output.closest[0] = Y;
                    output.closest[1] = Y;
                    return output;
                }
            }

            // Either (1) the line is not parallel to the rectangle and the
            // point of intersection of the line and the plane of the
            // rectangle is outside the rectangle or (2) the line and
            // rectangle are parallel. Regardless, the closest point on the
            // rectangle is on an edge of the rectangle. Compare the line to
            // all four edges of the rectangle. To allow for arbitrary
            // precision arithmetic, the initial distance and sqrDistance are
            // initialized to a negative number rather than a floating-point
            // maximum value. Tracking the minimum requires a small amount of
            // extra logic.

            using LSQuery = DCPQuery<T, Line3<T>, Segment3<T>>;
            LSQuery lsQuery{};
            typename LSQuery::Output lsOutput{};
            Segment3<T> segment{};

            T const invalid = -C_<T>(1);
            output.distance = invalid;
            output.sqrDistance = invalid;

            std::array<T, 4> const sign{ -C_<T>(1), C_<T>(1), -C_<T>(1), C_<T>(1) };
            std::array<size_t, 4> j0{ 0, 0, 1, 1 };
            std::array<size_t, 4> j1{ 1, 1, 0, 0 };
            std::array<std::array<size_t, 2>, 4> const edges
            {{
                // horizontal edges (y = -e1, +e1)
                { 0, 1 }, { 2, 3 },
                // vertical edges (x = -e0, +e0)
                { 0, 2 }, { 1, 3 }
            }};
            std::array<Vector3<T>, 4> vertices{};

            rectangle.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                lsOutput = lsQuery(line, segment);
                if (output.sqrDistance == invalid ||
                    lsOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = lsOutput.distance;
                    output.sqrDistance = lsOutput.sqrDistance;
                    output.parameter = lsOutput.parameter[0];
                    output.closest = lsOutput.closest;

                    T const scale = C_<T>(2) * lsOutput.parameter[1] - C_<T>(1);
                    output.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                    output.cartesian[j1[i]] = sign[i]  * rectangle.extent[j1[i]];
                }
            }

            return output;
        }
    };
}
