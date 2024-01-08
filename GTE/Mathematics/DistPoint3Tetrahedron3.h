// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

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

#include <Mathematics/DistPointTriangle.h>
#include <Mathematics/Tetrahedron3.h>
#include <array>
#include <cmath>
#include <cstddef>

namespace gte
{
    template <typename T>
    class DCPQuery<T, Vector3<T>, Tetrahedron3<T>>
    {
    public:
        struct Result
        {
            Result()
                :
                distance(static_cast<T>(0)),
                sqrDistance(static_cast<T>(0)),
                barycentric{ static_cast<T>(0), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0) },
                closest{ Vector3<T>::Zero(), Vector3<T>::Zero() }
            {
            }

            T distance, sqrDistance;
            std::array<T, 4> barycentric;
            std::array<Vector3<T>, 2> closest;
        };

        Result operator()(Vector3<T> const& point, Tetrahedron3<T> const& tetrahedron)
        {
            Result result{};

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
            T const invalid = static_cast<T>(-1);
            result.sqrDistance = invalid;
            for (size_t i = 0; i < 4; ++i)
            {
                if (Dot(planes[i].normal, point) >= planes[i].constant)
                {
                    auto const& indices = tetrahedron.GetFaceIndices(i);
                    Triangle3<T> triangle(
                        tetrahedron.v[indices[0]],
                        tetrahedron.v[indices[1]],
                        tetrahedron.v[indices[2]]);

                    DCPQuery<T, Vector3<T>, Triangle3<T>> ptQuery{};
                    auto ptResult = ptQuery(point, triangle);
                    if (result.sqrDistance == invalid ||
                        ptResult.sqrDistance < result.sqrDistance)
                    {
                        result.sqrDistance = ptResult.sqrDistance;
                        result.closest = ptResult.closest;
                    }
                }
            }

            T const zero = static_cast<T>(0);
            if (result.sqrDistance == invalid)
            {
                // The query point is inside the solid tetrahedron. Report a
                // zero distance. The closest points are identical.
                result.sqrDistance = zero;
                result.closest[0] = point;
                result.closest[1] = point;
            }
            result.distance = std::sqrt(result.sqrDistance);

            std::array<T, 4> barycentric{};
            (void)ComputeBarycentrics(result.closest[1], tetrahedron.v[0],
                tetrahedron.v[1], tetrahedron.v[2], tetrahedron.v[3],
                barycentric);

            return result;
        }
    };
}
