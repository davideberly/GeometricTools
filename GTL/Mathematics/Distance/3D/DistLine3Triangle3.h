// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a line and a solid triangle in 3D.
// 
// The line is P + t * D, where D is not required to be unit length.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on the line is stored in closest[0] with parameter t. The
// closest point on the triangle is closest[1] with barycentric coordinates
// (b[0],b[1],b[2]). When there are infinitely many choices for the pair of
// closest points, only one of them is returned.

#include <GTL/Mathematics/Distance/ND/DistLineSegment.h>
#include <GTL/Mathematics/Primitives/ND/Triangle.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Triangle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                parameter(C_<T>(0)),
                barycentric{ C_<T>(0), C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<T, 3> barycentric;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Line3<T> const& line, Triangle3<T> const& triangle)
        {
            // The line points are X = P + t * D and the triangle points
            // are Y = b[0] * V[0] + b[1] * V[1] + b[2] * V[2], where the
            // barycentric coordinates satisfy b[i] in [0,1] and
            // b[0] + b[1] + b[2 = 1. Define the triangle edge directions by
            // E[1] = V[1] - V[0] and E[2] = V[2] - V[0]; then
            // Y = V[0] + b1 * E[1] + b2 * E[2]. If Y is specified the
            // barycentric coordinates are the solution to
            //
            // +-                        -+ +-    -+   +-                 -+
            // | Dot(E1, E1)  Dot(E1, E2) | | b[1] | = | Dot(E1, Y - V[0]) |
            // | Dot(E1, E2)  Dot(E2, E2) | | b[2] |   | Dot(E2, Y - V[0]) |
            // +-                        -+ +-    -+   +-                 -+
            //
            // and b[0] = 1 - b[1] - b[2].

            Output output{};

            // Test whether the line intersects triangle. If so, the squared
            // distance is zero. The normal of the plane of the triangle does
            // not have to be normalized to unit length.
            Vector3<T> E1 = triangle.v[1] - triangle.v[0];
            Vector3<T> E2 = triangle.v[2] - triangle.v[0];
            Vector3<T> N = Cross(E1, E2);
            T NdD = Dot(N, line.direction);
            if (std::fabs(NdD) > C_<T>(0))
            {
                // The line and triangle are not parallel, so the line
                // intersects the plane of the triangle at a point Y.
                // Determine whether Y is contained by the triangle.
                Vector3<T> PmV0 = line.origin - triangle.v[0];
                T NdDiff = Dot(N, PmV0);
                T tIntersect = -NdDiff / NdD;
                Vector3<T> Y = line.origin + tIntersect * line.direction;
                Vector3<T> YmV0 = Y - triangle.v[0];

                // Compute the barycentric coordinates of the intersection.
                T E1dE1 = Dot(E1, E1);
                T E1dE2 = Dot(E1, E2);
                T E2dE2 = Dot(E2, E2);
                T E1dYmV0 = Dot(E1, YmV0);
                T E2dYmV0 = Dot(E2, YmV0);
                T det = E1dE1 * E2dE2 - E1dE2 * E1dE2;
                T b1 = (E2dE2 * E1dYmV0 - E1dE2 * E2dYmV0) / det;
                T b2 = (E1dE1 * E2dYmV0 - E1dE2 * E1dYmV0) / det;
                T b0 = C_<T>(1) - b1 - b2;

                if (b0 >= C_<T>(0) && b1 >= C_<T>(0) && b2 >= C_<T>(0))
                {
                    // The point Y is contained by the triangle.
                    output.distance = C_<T>(0);
                    output.sqrDistance = C_<T>(0);
                    output.parameter = tIntersect;
                    output.barycentric[0] = b0;
                    output.barycentric[1] = b1;
                    output.barycentric[2] = b2;
                    output.closest[0] = Y;
                    output.closest[1] = Y;
                    return output;
                }
            }

            // Either (1) the line is not parallel to the triangle and the
            // point of intersection of the line and the plane of the triangle
            // is outside the triangle or (2) the line and triangle are
            // parallel. Regardless, the closest point on the triangle is on
            // an edge of the triangle. Compare the line to all three edges
            // of the triangle. To allow for arbitrary precision arithmetic,
            // the initial distance and sqrDistance are initialized to a
            // negative number rather than a floating-point maximum value.
            // Tracking the minimum requires a small amount of extra logic.
            T const invalid = -C_<T>(1);
            output.distance = invalid;
            output.sqrDistance = invalid;

            using LSQuery = DCPQuery<T, Line3<T>, Segment3<T>>;
            LSQuery lsQuery{};
            typename LSQuery::Output lsOutput{};
            Segment3<T> segment{};

            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle.v[i0];
                segment.p[1] = triangle.v[i1];

                lsOutput = lsQuery(line, segment);
                if (output.sqrDistance == invalid ||
                    lsOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = lsOutput.distance;
                    output.sqrDistance = lsOutput.sqrDistance;
                    output.parameter = lsOutput.parameter[0];
                    output.barycentric[i0] = C_<T>(1) - lsOutput.parameter[1];
                    output.barycentric[i1] = lsOutput.parameter[1];
                    output.barycentric[i2] = C_<T>(0);
                    output.closest = lsOutput.closest;
                }
            }

            return output;
        }
    };
}
