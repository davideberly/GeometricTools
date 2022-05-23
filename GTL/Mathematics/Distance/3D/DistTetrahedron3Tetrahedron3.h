// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

#include <GTL/Mathematics/Distance/3D/DistTriangle3Triangle3.h>
#include <GTL/Mathematics/Containment/3D/ContTetrahedron3.h>
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

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Tetrahedron3<T>, Tetrahedron3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                barycentric0{ C_<T>(0), C_<T>(0), C_<T>(0), C_<T>(0) },
                barycentric1{ C_<T>(0), C_<T>(0), C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 4> barycentric0;
            std::array<T, 4> barycentric1;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Tetrahedron3<T> const& tetra0, Tetrahedron3<T> const& tetra1)
        {
            Output output{};

            DCPQuery<T, Triangle3<T>, Triangle3<T>> ttQuery{};
            typename DCPQuery<T, Triangle3<T>, Triangle3<T>>::Output ttResult{};
            Triangle3<T> triangle{};

            T const invalid = -C_<T>(1);
            output.distance = invalid;
            output.sqrDistance = invalid;

            // Compute the distances between pairs of faces, each pair having
            // a face from tetra0 and a face from tetra1.
            bool foundZeroDistance = false;
            for (size_t face0 = 0; face0 < 4; ++face0)
            {
                Triangle3<T> triangle0{};
                std::array<size_t, 3> indices0 =
                    Tetrahedron3<T>::GetFaceIndices(face0);

                for (size_t j = 0; j < 3; ++j)
                {
                    triangle0.v[j] = tetra0.v[indices0[j]];
                }

                for (size_t face1 = 0; face1 < 4; ++face1)
                {
                    Triangle3<T> triangle1{};
                    std::array<size_t, 3> indices1 =
                        Tetrahedron3<T>::GetFaceIndices(face1);

                    for (size_t j = 0; j < 3; ++j)
                    {
                        triangle1.v[j] = tetra1.v[indices1[j]];
                    }

                    ttResult = ttQuery(triangle0, triangle1);
                    if (ttResult.sqrDistance == C_<T>(0))
                    {
                        output.distance = C_<T>(0);
                        output.sqrDistance = C_<T>(0);
                        output.closest[0] = ttResult.closest[0];
                        output.closest[1] = ttResult.closest[1];
                        foundZeroDistance = true;
                        break;
                    }

                    if (output.sqrDistance == invalid ||
                        ttResult.sqrDistance < output.sqrDistance)
                    {
                        output.distance = ttResult.distance;
                        output.sqrDistance = ttResult.sqrDistance;
                        output.closest[0] = ttResult.closest[0];
                        output.closest[1] = ttResult.closest[1];
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
                    output.distance = C_<T>(0);
                    output.sqrDistance = C_<T>(0);
                    output.closest[0] = centroid0;
                    output.closest[1] = centroid0;
                }

                Vector3<T> centroid1 = tetra1.ComputeCentroid();
                bool centroid1InTetra0 = InContainer(centroid1, tetra0);
                if (centroid1InTetra0)
                {
                    // Tetra1 is nested inside tetra0. Choose the centroid
                    // of tetra1 as the closest point for both tetrahedra.
                    output.distance = C_<T>(0);
                    output.sqrDistance = C_<T>(0);
                    output.closest[0] = centroid1;
                    output.closest[1] = centroid1;
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
            (void)ComputeBarycentrics(output.closest[0],
                tetra0.v[0], tetra0.v[1], tetra0.v[2], tetra0.v[3], C_<T>(0),
                output.barycentric0);

            (void)ComputeBarycentrics(output.closest[1],
                tetra1.v[0],  tetra1.v[1], tetra1.v[2], tetra1.v[3], C_<T>(0),
                output.barycentric1);

            return output;
        }
    };
}
