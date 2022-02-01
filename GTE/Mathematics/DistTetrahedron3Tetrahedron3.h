// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.02.01

#pragma once

#include <Mathematics/DistTriangle3Triangle3.h>
#include <Mathematics/ContTetrahedron3.h>
#include <array>

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
                std::array<size_t, 3> indices0{};
                Tetrahedron3<T>::GetFaceIndices(face0, indices0);
                for (size_t j = 0; j < 3; ++j)
                {
                    triangle0.v[j] = tetra0.v[indices0[j]];
                }

                for (size_t face1 = 0; face1 < 4; ++face1)
                {
                    Triangle3<T> triangle1{};
                    std::array<size_t, 3> indices1{};
                    Tetrahedron3<T>::GetFaceIndices(face1, indices1);
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
                // The tetrahedra are either nested or separated. Let cv0 be
                // the number of vertices of tetra0 strictly inside tetra1
                // and let cv1 be the number of vertices of tetra1 strictly
                // inside tetra0. In theory, cv0 = 4 and cv1 = 0 (tetra0 is
                // nested inside tetra1) or cv0 = 0 and cv1 = 4 (tetra1 is
                // nested inside tetra0) or cv0 = 0 and cv1 = 0 (tetra0 and
                // tetra1 are separated). When using floating-point
                // arithmetic, counts of contained points are used in case
                // of rounding errors. If cv0 = 0 and cv1 = 0, the tetrahedra
                // are separated; otherwise, the fuzzy test for nesting is
                // cv0 > cv1 or cv1 > cv0.
                size_t cv0 = 0;
                for (size_t j = 0; j < 4; ++j)
                {
                    if (InContainer(tetra0.v[j], tetra1))
                    {
                        ++cv0;
                    }
                }

                size_t cv1 = 0;
                for (size_t j = 0; j < 4; ++j)
                {
                    if (InContainer(tetra1.v[j], tetra0))
                    {
                        ++cv1;
                    }
                }

                if (cv0 != 0 || cv1 != 0)
                {
                    // One tetrahedra is nested inside the other.
                    result.distance = zero;
                    result.sqrDistance = zero;

                    if (cv0 > 0 && cv1 == 0)
                    {
                        // The tetra0 is nested inside tetra1. Choose the
                        // centroid of tetra0 as the closest point for both
                        // tetrahedra.
                        Vector3<T> centroid0 = tetra0.ComputeCentroid();
                        result.closest[0] = centroid0;
                        result.closest[1] = centroid0;
                    }
                    else if (cv0 == 0 && cv1 > 0)
                    {
                        // The tetra1 is nested inside tetra0. Choose the
                        // centroid of tetra1 as the closest point for both
                        // tetrahedra.
                        Vector3<T> centroid1 = tetra1.ComputeCentroid();
                        result.closest[0] = centroid1;
                        result.closest[1] = centroid1;
                    }
                    else  // cv0 > 0 and cv1 > 0
                    {
                        // Rounding errors occurred in the point-tetrahedron
                        // containment query.
                        if (cv0 > cv1)
                        {
                            // The tetra0 is assumed to be nested inside
                            // tetra1. Choose the centroid of tetra0 as the
                            // closest point for both tetrahedra.
                            Vector3<T> centroid0 = tetra0.ComputeCentroid();
                            result.closest[0] = centroid0;
                            result.closest[1] = centroid0;
                        }
                        else if (cv1 > cv0)
                        {
                            // The tetra1 is assumed to be nested inside
                            // tetra0. Choose the centroid of tetra1 as the
                            // closest point for both tetrahedra.
                            Vector3<T> centroid1 = tetra1.ComputeCentroid();
                            result.closest[0] = centroid1;
                            result.closest[1] = centroid1;
                        }
                        else  // cv0 = cv1
                        {
                            // Numerically this can occur if the tetrahedra
                            // are (nearly) the same. Choose the closest point
                            // to be the average of the centroids.
                            Vector3<T> centroid0 = tetra0.ComputeCentroid();
                            Vector3<T> centroid1 = tetra1.ComputeCentroid();
                            Vector3<T> average = (centroid0 + centroid1) * static_cast<T>(0.5);
                            result.closest[0] = average;
                            result.closest[1] = average;
                        }
                    }
                }
                // else: The tetrahedra are separated. The result already
                // stores the distance, sqrDistance and closest[] values.
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
