// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between two rectangles in 3D.
//
// Each rectangle has center C, unit-length axis directions W[0] and W[1], and
// extents e[0] and e[1]. A rectangle point is X = C + sum_{i=0}^2 s[i] * W[i]
// where |s[i]| <= e[i] for all i.
// 
// The closest point on rectangle0 is stored in closest[0] with W-coordinates
// (s[0],s[1]) corresponding to its W-axes. The closest point on rectangle1 is
// stored in closest[1] with W-coordinates (s[0],s[1]) corresponding to its
// W-axes. When there are infinitely many choices for the pair of closest
// points, only one of them is returned.
//
// TODO: Modify to support non-unit-length W[].

#include <GTL/Mathematics/Distance/3D/DistSegment3Rectangle3.h>
#include <array>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, Rectangle3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                cartesian0{ C_<T>(0) , C_<T>(0) },
                cartesian1{ C_<T>(0) , C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> cartesian0;
            std::array<T, 2> cartesian1;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Rectangle3<T> const& rectangle0, Rectangle3<T> const& rectangle1)
        {
            Output output{};

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

            // Compare edges of rectangle0 to the interior of rectangle1.
            rectangle0.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                srOutput = srQuery(segment, rectangle1);
                if (output.sqrDistance == invalid ||
                    srOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = srOutput.distance;
                    output.sqrDistance = srOutput.sqrDistance;
                    T const scale = C_<T>(2) * srOutput.parameter - C_<T>(1);
                    output.cartesian0[j0[i]] = scale * rectangle0.extent[j0[i]];
                    output.cartesian0[j1[i]] = sign[i] * rectangle0.extent[j1[i]];
                    output.cartesian1 = srOutput.cartesian;
                    output.closest = srOutput.closest;
                }
            }

            // Compare edges of rectangle1 to the interior of rectangle0.
            rectangle1.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                srOutput = srQuery(segment, rectangle0);
                if (output.sqrDistance == invalid ||
                    srOutput.sqrDistance < output.sqrDistance)
                {
                    output.distance = srOutput.distance;
                    output.sqrDistance = srOutput.sqrDistance;
                    T const scale = C_<T>(2) * srOutput.parameter - C_<T>(1);
                    output.cartesian0 = srOutput.cartesian;
                    output.cartesian1[j0[i]] = scale * rectangle1.extent[j0[i]];
                    output.cartesian1[j1[i]] = sign[i] * rectangle1.extent[j1[i]];
                    output.closest[0] = srOutput.closest[1];
                    output.closest[1] = srOutput.closest[0];
                }
            }

            return output;
        }
    };
}
