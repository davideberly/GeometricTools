// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

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

#include <GTL/Mathematics/Distance/3D/DistSegment3Rectangle3.h>
#include <GTL/Mathematics/Distance/3D/DistSegment3Triangle3.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, Rectangle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                barycentric{ C_<T>(0), C_<T>(0), C_<T>(0) },
                cartesian{ C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric;
            std::array<T, 2> cartesian;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Triangle3<T> const& triangle, Rectangle3<T> const& rectangle)
        {
            Output output{};

            DCPQuery<T, Segment3<T>, Triangle3<T>> stQuery{};
            typename DCPQuery<T, Segment3<T>, Triangle3<T>>::Output stOutput{};
            DCPQuery<T, Segment3<T>, Rectangle3<T>> srQuery{};
            typename DCPQuery<T, Segment3<T>, Rectangle3<T>>::Output srOutput{};
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

            // Compare edges of the triangle to the interior of rectangle.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle.v[i0];
                segment.p[1] = triangle.v[i1];

                srOutput = srQuery(segment, rectangle);
                if (output.sqrDistance == invalid ||
                    srOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = srOutput.distance;
                    output.sqrDistance = srOutput.sqrDistance;
                    output.barycentric[i0] = C_<T>(1) - srOutput.parameter;
                    output.barycentric[i1] = srOutput.parameter;
                    output.barycentric[i2] = C_<T>(0);
                    output.cartesian = srOutput.cartesian;
                    output.closest = srOutput.closest;
                }
            }

            // Compare edges of the rectangle to the interior of triangle.
            rectangle.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                stOutput = stQuery(segment, triangle);
                if (output.sqrDistance == invalid ||
                    stOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = stOutput.distance;
                    output.sqrDistance = stOutput.sqrDistance;
                    output.barycentric = stOutput.barycentric;
                    T const scale = C_<T>(2) * stOutput.parameter - C_<T>(1);
                    output.cartesian[j0[i]] = scale * rectangle.extent[j0[i]];
                    output.cartesian[j1[i]] = sign[i] * rectangle.extent[j1[i]];
                    output.closest[0] = stOutput.closest[1];
                    output.closest[1] = stOutput.closest[0];
                }
            }
            return output;
        }
    };
}
