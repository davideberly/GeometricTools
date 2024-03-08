// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between a solid triangle and a solid rectangle in 3D.
// 
// The triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The closest point on the triangle is stored in closest[0] with barycentric
// coordinates (b[0],b[1],b[2]). The closest point on the rectangle is stoed
// closest[1] with cartesian[] coordinates (s[0],s[1]). When there are
// infinitely many choices for the pair of closest points, only one of them is
// returned.
//
// TODO: Modify to support non-unit-length W[].

#include <Mathematics/DistSegment3Rectangle3.h>
#include <Mathematics/DistSegment3Triangle3.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, Rectangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                cartesian{ static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Triangle3<T> const& triangle, Rectangle3<T> const& rectangle)
        {
            Result result{};

            DCPQuery<T, Segment3<T>, Triangle3<T>> stQuery{};
            typename DCPQuery<T, Segment3<T>, Triangle3<T>>::Result stResult{};
            DCPQuery<T, Segment3<T>, Rectangle3<T>> srQuery{};
            typename DCPQuery<T, Segment3<T>, Rectangle3<T>>::Result srResult{};
            Segment3<T> segment{};

            T const zero = static_cast<T>(0);
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
                // horizontal edges (y = -e1, +e1)
                { 0, 1 }, { 2, 3 },
                // vertical edges (x = -e0, +e0)
                { 0, 2 }, { 1, 3 }
            }};
            std::array<Vector3<T>, 4> vertices{};

            // Compare edges of triangle to the interior of rectangle.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle.v[i0];
                segment.p[1] = triangle.v[i1];

                srResult = srQuery(segment, rectangle);
                if (result.sqrDistance == invalid ||
                    srResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = srResult.distance;
                    result.sqrDistance = srResult.sqrDistance;
                    result.barycentric[i0] = one - srResult.parameter;
                    result.barycentric[i1] = srResult.parameter;
                    result.barycentric[i2] = zero;
                    result.cartesian = srResult.cartesian;
                    result.closest = srResult.closest;
                }
            }

            // Compare edges of rectangle to the interior of triangle.
            rectangle.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                stResult = stQuery(segment, triangle);
                if (result.sqrDistance == invalid ||
                    stResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = stResult.distance;
                    result.sqrDistance = stResult.sqrDistance;
                    result.barycentric = stResult.barycentric;
                    T const scale = two * stResult.parameter - one;
                    result.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                    result.cartesian[j1[i]] = sign[i] * rectangle.extent[j1[i]];
                    result.closest[0] = stResult.closest[1];
                    result.closest[1] = stResult.closest[0];
                }
            }
            return result;
        }
    };
}
