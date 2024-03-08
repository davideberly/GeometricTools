// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistSegment3Rectangle3.h>
#include <array>
#include <cstddef>
#include <cstdint>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Rectangle3<T>, Rectangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                cartesian0{ static_cast<T>(0) , static_cast<T>(0) },
                cartesian1{ static_cast<T>(0) , static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 2> cartesian0;
            std::array<T, 2> cartesian1;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Rectangle3<T> const& rectangle0, Rectangle3<T> const& rectangle1)
        {
            Result result{};

            DCPQuery<T, Segment3<T>, Rectangle3<T>> srQuery{};
            typename DCPQuery<T, Segment3<T>, Rectangle3<T>>::Result srResult{};
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

                srResult = srQuery(segment, rectangle1);
                if (result.sqrDistance == invalid ||
                    srResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = srResult.distance;
                    result.sqrDistance = srResult.sqrDistance;
                    T const scale = two * srResult.parameter - one;
                    result.cartesian0[j0[i]] = scale * rectangle0.extent[j0[i]];
                    result.cartesian0[j1[i]] = sign[i] * rectangle0.extent[j1[i]];
                    result.cartesian1 = srResult.cartesian;
                    result.closest = srResult.closest;
                }
            }

            // Compare edges of rectangle1 to the interior of rectangle0.
            rectangle1.GetVertices(vertices);
            for (size_t i = 0; i < 4; ++i)
            {
                auto const& edge = edges[i];
                segment.p[0] = vertices[edge[0]];
                segment.p[1] = vertices[edge[1]];

                srResult = srQuery(segment, rectangle0);
                if (result.sqrDistance == invalid ||
                    srResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = srResult.distance;
                    result.sqrDistance = srResult.sqrDistance;
                    T const scale = two * srResult.parameter - one;
                    result.cartesian0 = srResult.cartesian;
                    result.cartesian1[j0[i]] = scale * rectangle1.extent[j0[i]];
                    result.cartesian1[j1[i]] = sign[i] * rectangle1.extent[j1[i]];
                    result.closest[0] = srResult.closest[1];
                    result.closest[1] = srResult.closest[0];
                }
            }

            return result;
        }
    };
}
