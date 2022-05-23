// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.22

#pragma once

// Compute the distance between a point and a solid tetrahedron in 3D.
// 
// The tetrahedron is represented as an array of four vertices, V[i] for
// 0 <= i <= 3. The vertices are ordered so that the triangular faces are
// counterclockwise-ordered triangles when viewed by an observer outside the
// tetrahedron: face 0 = <V[0],V[2],V[1]>, face 1 = <V[0],V[1],V[3]>,
// face 2 = <V[0],V[3],V[2]> and face 3 = <V[1],V[2],V[3]>. The canonical
// tetrahedron has V[0] = (0,0,0), V[1] = (1,0,0), V[2] = (0,1,0) and
// V[3] = (0,0,1). A tetrahedron point is // X = sum_{i=0}^3 b[i] * V[i],
// where 0 <= b[i] <= 1 for all i and sum_{i=0}^3 b[i] = 1.
// 
// The input P is stored in closest[0]. The closest point on the tetrahedron
// is stored in closest[1] with barycentric coordinates (b[0],b[1],b[2],b[3]).

#include <GTL/Mathematics/Distance/ND/DistPointTriangle.h>
#include <GTL/Mathematics/Primitives/3D/Tetrahedron3.h>
#include <array>
#include <cmath>

namespace gtl
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Tetrahedron3<T>>
    {
    public:
        struct Output
        {
            Output()
                :
                distance(C_<T>(0)),
                sqrDistance(C_<T>(0)),
                barycentric{ C_<T>(0), C_<T>(0), C_<T>(0), C_<T>(0) },
                closest{}
            {
            }

            T distance, sqrDistance;
            std::array<T, 4> barycentric;
            std::array<Vector3<T>, 2> closest;
        };

        Output operator()(Vector3<T> const& point, Tetrahedron3<T> const& tetrahedron)
        {
            Output output{};

            // Construct the planes for the faces of the tetrahedron. The
            // normals are outer pointing, but specified not to be unit
            // length. We only need to know sidedness of the query point, so
            // we will save cycles by not computing unit-length normals.
            std::array<Plane3<T>, 4> planes{};
            tetrahedron.GetPlanes(planes);

            // Determine which faces are visible to the query point. Only
            // these need to be processed by point-to-triangle distance
            // queries. To allow for arbitrary precision arithmetic, the
            // initial sqrDistance is initialized to zero rather than a
            // floating-point maximum value. Tracking the minimum requires a
            // small amount of extra logic.
            T const invalid = -C_<T>(1);
            output.sqrDistance = invalid;
            for (size_t i = 0; i < 4; ++i)
            {
                if (Dot(planes[i].normal, point) >= planes[i].constant)
                {
                    std::array<size_t, 3> indices =
                        Tetrahedron3<T>::GetFaceIndices(i);

                    Triangle3<T> triangle(
                        tetrahedron.v[indices[0]],
                        tetrahedron.v[indices[1]],
                        tetrahedron.v[indices[2]]);

                    DCPQuery<T, Vector3<T>, Triangle3<T>> ptQuery{};
                    auto ptResult = ptQuery(point, triangle);
                    if (output.sqrDistance == invalid ||
                        ptResult.sqrDistance < output.sqrDistance)
                    {
                        output.sqrDistance = ptResult.sqrDistance;
                        output.closest = ptResult.closest;
                    }
                }
            }

            if (output.sqrDistance == invalid)
            {
                // The query point is inside the solid tetrahedron. Report a
                // zero distance. The closest points are identical.
                output.sqrDistance = C_<T>(0);
                output.closest[0] = point;
                output.closest[1] = point;
            }
            output.distance = std::sqrt(output.sqrDistance);

            ComputeBarycentrics(output.closest[1], tetrahedron.v[0],
                tetrahedron.v[1], tetrahedron.v[2], tetrahedron.v[3],
                C_<T>(0), output.barycentric);

            return output;
        }
    };
}
