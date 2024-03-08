// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2023.08.08

#pragma once

// Compute the distance between two solid tetrahedra in 3D.
// 
// Each tetrahedron has vertices <V[0],V[1],V[2],V[3]>. A tetrahedron point
// is X = sum_{i=0}^3 b[i] * V[i], where 0 <= b[i] <= 1 for all i and
// sum_{i=0}^3 b[i] = 1.
// 
// The closest point on tetra0 is stored in closest[0] with barycentric
// coordinates relative to its vertices. The closest point on tetra1 is stored
// in closest[1] with barycentric coordinates relative to its vertices. When
// there are infinitely many choices for the pair of closest points, only one
// pair is returned.

#include <Mathematics/DistTriangle3Triangle3.h>
#include <Mathematics/ContTetrahedron3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Tetrahedron3<T>, Tetrahedron3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric0{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                barycentric1{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 4> barycentric0;
            std::array<T, 4> barycentric1;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Tetrahedron3<T> const& tetra0, Tetrahedron3<T> const& tetra1)
        {
            Result result{};

            DCPQuery<T, Triangle3<T>, Triangle3<T>> ttQuery{};
            typename DCPQuery<T, Triangle3<T>, Triangle3<T>>::Result ttResult{};
            Triangle3<T> triangle{};

            T const zero = static_cast<T>(0);
            T const invalid = static_cast<T>(-1);
            result.distance = invalid;
            result.sqrDistance = invalid;

            // Compute the distances between pairs of faces, each pair having
            // a face from tetra0 and a face from tetra1.
            bool foundZeroDistance = false;
            for (size_t face0 = 0; face0 < 4; ++face0)
            {
                Triangle3<T> triangle0{};
                auto const& indices0 = Tetrahedron3<T>::GetFaceIndices(face0);
                for (size_t j = 0; j < 3; ++j)
                {
                    triangle0.v[j] = tetra0.v[indices0[j]];
                }

                for (size_t face1 = 0; face1 < 4; ++face1)
                {
                    Triangle3<T> triangle1{};
                    auto const& indices1 = Tetrahedron3<T>::GetFaceIndices(face1);
                    for (size_t j = 0; j < 3; ++j)
                    {
                        triangle1.v[j] = tetra1.v[indices1[j]];
                    }

                    ttResult = ttQuery(triangle0, triangle1);
                    if (ttResult.sqrDistance == zero)
                    {
                        result.distance = zero;
                        result.sqrDistance = zero;
                        result.closest[0] = ttResult.closest[0];
                        result.closest[1] = ttResult.closest[1];
                        foundZeroDistance = true;
                        break;
                    }

                    if (result.sqrDistance == invalid ||
                        ttResult.sqrDistance < result.sqrDistance)
                    {
                        result.distance = ttResult.distance;
                        result.sqrDistance = ttResult.sqrDistance;
                        result.closest[0] = ttResult.closest[0];
                        result.closest[1] = ttResult.closest[1];
                    }
                }

                if (foundZeroDistance)
                {
                    break;
                }
            }

            if (!foundZeroDistance)
            {
                // The tetrahedra are either nested or separated. Test
                // for containment of the centroids to decide which case.
                Vector3<T> centroid0 = tetra0.ComputeCentroid();
                bool centroid0InTetra1 = InContainer(centroid0, tetra1);
                if (centroid0InTetra1)
                {
                    // Tetra0 is nested inside tetra1. Choose the centroid
                    // of tetra0 as the closest point for both tetrahedra.
                    result.distance = zero;
                    result.sqrDistance = zero;
                    result.closest[0] = centroid0;
                    result.closest[1] = centroid0;
                }

                Vector3<T> centroid1 = tetra1.ComputeCentroid();
                bool centroid1InTetra0 = InContainer(centroid1, tetra0);
                if (centroid1InTetra0)
                {
                    // Tetra1 is nested inside tetra0. Choose the centroid
                    // of tetra1 as the closest point for both tetrahedra.
                    result.distance = zero;
                    result.sqrDistance = zero;
                    result.closest[0] = centroid1;
                    result.closest[1] = centroid1;
                }

                // With exact arithmetic, at this point the tetrahedra are
                // separated. The output object already contains the distance
                // information. However, with floating-point arithmetic, it
                // is possible that a tetrahedron with volume nearly zero is
                // close enough to the other tetrahedron yet separated, but
                // rounding errors make it appear that the nearly-zero-volume
                // tetrahedron has centroid inside the other tetrahedron. This
                // situation is trapped by the previous two if-blocks.
            }

            // Compute the barycentric coordinates of the closest points.
            (void)ComputeBarycentrics(result.closest[0],
                tetra0.v[0], tetra0.v[1], tetra0.v[2], tetra0.v[3],
                result.barycentric0);

            (void)ComputeBarycentrics(result.closest[1],
                tetra1.v[0],  tetra1.v[1], tetra1.v[2], tetra1.v[3],
                result.barycentric1);

            return result;
        }
    };
}
