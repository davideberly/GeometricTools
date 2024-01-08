// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// Compute the distance between two solid triangles in 3D.
// 
// Each triangle has vertices <V[0],V[1],V[2]>. A triangle point is
// X = sum_{i=0}^2 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^2 b[i] = 1.
// 
// The closest point on triangle0 is stored in closest[0] with barycentric
// coordinates relative to its vertices. The closest point on triangle1 is
// stored in closest[1] with barycentric coordinates relative to its vertices.
// When there are infinitely many choices for the pair of closest points, only
// one pair is returned.

#include <Mathematics/DistSegment3Triangle3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Triangle3<T>, Triangle3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric0{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                barycentric1{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 3> barycentric0;
            std::array<T, 3> barycentric1;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Triangle3<T> const& triangle0, Triangle3<T> const& triangle1)
        {
            Result result{};

            DCPQuery<T, Segment3<T>, Triangle3<T>> stQuery{};
            typename DCPQuery<T, Segment3<T>, Triangle3<T>>::Result stResult;
            Segment3<T> segment{};

            T const zero = static_cast<T>(0);
            T const one = static_cast<T>(1);
            T const invalid = static_cast<T>(-1);
            result.distance = invalid;
            result.sqrDistance = invalid;

            // Compare edges of triangle0 to the interior of triangle1.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle0.v[i0];
                segment.p[1] = triangle0.v[i1];

                stResult = stQuery(segment, triangle1);
                if (result.sqrDistance == invalid ||
                    stResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = stResult.distance;
                    result.sqrDistance = stResult.sqrDistance;
                    result.barycentric0[i0] = one - stResult.parameter;
                    result.barycentric0[i1] = stResult.parameter;
                    result.barycentric0[i2] = zero;
                    result.barycentric1 = stResult.barycentric;
                    result.closest = stResult.closest;
                }
            }

            // Compare edges of triangle1 to the interior of triangle0.
            for (size_t i0 = 2, i1 = 0, i2 = 1; i1 < 3; i2 = i0, i0 = i1++)
            {
                segment.p[0] = triangle1.v[i0];
                segment.p[1] = triangle1.v[i1];

                stResult = stQuery(segment, triangle0);
                if (result.sqrDistance == invalid ||
                    stResult.sqrDistance < result.sqrDistance)
                {
                    result.distance = stResult.distance;
                    result.sqrDistance = stResult.sqrDistance;
                    result.barycentric0 = stResult.barycentric;
                    result.barycentric1[i0] = one - stResult.parameter;
                    result.barycentric1[i1] = stResult.parameter;
                    result.barycentric1[i2] = zero;
                    result.closest[0] = stResult.closest[1];
                    result.closest[1] = stResult.closest[0];
                }
            }
            return result;
        }
    };
}
