// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistLineSegment.h>
#include <Mathematics/Rectangle.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Line3<T>, Rectangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                parameter(static_cast<T>(0)),
                cartesian{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            T parameter;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Line3<T> const& line, Rectangle3<T> const& rectangle)
        {
            Result result{};

            // Test whether the line intersects rectangle. If so, the squared
            // distance is zero. The normal of the plane of the rectangle does
            // not have to be normalized to unit length.
            T const zero = static_cast<T>(0);
            Vector3<T> N = Cross(rectangle.axis[0], rectangle.axis[1]);
            T NdD = Dot(N, line.direction);
            if (std::fabs(NdD) > zero)
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
                    result.sqrDistance = zero;
                    result.distance = zero;
                    result.parameter = tIntersect;
                    result.cartesian[0] = s0;
                    result.cartesian[1] = s1;
                    result.closest[0] = Y;
                    result.closest[1] = Y;
                    return result;
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
            typename LSQuery::Result lsResult{};
            Segment3<T> segment{};

            T const one = static_cast<T>(1);
            T const negOne = static_cast<T>(-1);
            T const two = static_cast<T>(2);
            T const invalid = static_cast<T>(-1);
            result.distance = invalid;
            result.sqrDistance = invalid;

            std::array<T, 4> const sign{ negOne, one, negOne, one };
            std::array<int32_t, 4> j0{ 0, 0, 1, 1 };
            std::array<int32_t, 4> j1{ 1, 1, 0, 0 };
            std::array<std::array<size_t, 2>, 4> const edges
            {{
                // horizontal edges (y = +e1 or -e1)
                { 0, 1 }, { 2, 3 },
                // vertical edges (x = +e0 or -e0)
                { 0, 2 }, { 1, 3 }
            }};
            std::array<Vector3<T>, 4> vertices{};

            rectangle.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                lsResult = lsQuery(line, segment);
                if (result.sqrDistance == invalid ||
                    lsResult.sqrDistance < result.sqrDistance)
                {
                    result.sqrDistance = lsResult.sqrDistance;
                    result.distance = lsResult.distance;
                    result.parameter = lsResult.parameter[0];
                    result.closest = lsResult.closest;

                    T const scale = two * lsResult.parameter[1] - one;
                    result.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                    result.cartesian[j1[i]] = sign[i]  * rectangle.extent[j1[i]];
                }
            }

            return result;
        }
    };

    // Template alias for convenience.
    template <typename T>
    using DCPLine3Rectangle3 = DCPQuery<T, Line3<T>, Rectangle3<T>>;
}
