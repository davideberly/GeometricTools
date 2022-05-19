// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The convex polyhedra represented by this class have triangle faces that are
// counterclockwise ordered when viewed from outside the polyhedron. No
// attempt is made to verify that the polyhedron is convex; the caller is
// responsible for enforcing this. The constructor succeeds when the number of
// vertices is at least 4 and the number of indices is at least 12. If the
// constructor fails, no move occurs and the member arrays have no elements.
//
// To support geometric algorithms that are formulated using convex quadratic
// programming such as computing the distance from a point to a convex
// polyhedron, it is necessary to know the planes of the faces and an
// axis-aligned bounding box. If you want either the faces or the box, pass
// 'true' to the appropriate parameters. When planes are generated, the
// normals are not created to be unit length in order to support queries using
// exact rational arithmetic. If a normal to a face is N = (n0,n1,n2) and V is
// a vertex of the face, the plane is Dot(N,X-V) = 0 and is stored as
// Dot(n0,n1,n2,-Dot(N,V)). The normals are computed to be outer pointing.
// 
// Comparison operators are not provided. The semantics of equal polyhedra is
// complicated and (at the moment) not useful. The vertices of one polyhedron
// can be a permutation of the other polyhedron, but the polyhedra are the
// same geometrically. It is not clear how to implement an efficient
// comparison that does not process all possible permutations.

#include <GTL/Mathematics/Primitives/ND/AlignedBox.h>
#include <cstddef>
#include <vector>

namespace gtl
{
    template <typename T>
    class ConvexPolyhedron3
    {
    public:
        using value_type = T;

        ConvexPolyhedron3()
            :
            vertices{},
            indices{},
            planes{},
            alignedBox{}
        {
        }

        ConvexPolyhedron3(std::vector<Vector3<T>> const& inVertices,
            std::vector<size_t> const& inIndices, bool wantPlanes, bool wantAlignedBox)
            :
            vertices(inVertices),
            indices(inIndices),
            planes{},
            alignedBox{}
        {
            GTL_ARGUMENT_ASSERT(
                inVertices.size() >= 4 && inIndices.size() >= 12,
                "Invalid input.");

            if (wantPlanes)
            {
                GeneratePlanes();
            }

            if (wantAlignedBox)
            {
                GenerateAlignedBox();
            }
        }

        // If you modifty the vertices or indices and you want the new face
        // planes or aligned box computed, call these functions.
        void GeneratePlanes()
        {
            if (vertices.size() > 0 && indices.size() > 0)
            {
                size_t const numTriangles = indices.size() / 3;
                planes.resize(numTriangles);
                for (size_t t = 0, i = 0; t < numTriangles; ++t)
                {
                    Vector3<T> V0 = vertices[indices[i++]];
                    Vector3<T> V1 = vertices[indices[i++]];
                    Vector3<T> V2 = vertices[indices[i++]];
                    Vector3<T> E1 = V1 - V0;
                    Vector3<T> E2 = V2 - V0;
                    Vector3<T> N = Cross(E1, E2);
                    planes[t] = HLift(N, -Dot(N, V0));
                }
            }
        }

        void GenerateAlignedBox()
        {
            if (vertices.size() > 0 && indices.size() > 0)
            {
                auto extreme = ComputeExtremes(vertices);
                alignedBox.min = extreme.first;
                alignedBox.max = extreme.second;
            }
        }

        std::vector<Vector3<T>> vertices;
        std::vector<size_t> indices;
        std::vector<Vector4<T>> planes;
        AlignedBox3<T> alignedBox;

    private:
        friend class UnitTestConvexPolyhedron3;
    };
}
