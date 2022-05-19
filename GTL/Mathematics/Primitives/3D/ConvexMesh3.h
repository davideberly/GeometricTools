// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// A client of ConvexMesh3 is responsible for populating the vertices and
// indices so that the resulting mesh represents a convex polyhedron.
//   1. All elements of 'vertices' must be used by the polyhedron.
//   2. The triangle faces must have the same chirality when viewed from
//      outside the polyhedron. They are all counterclockwise oriented or all
//      clockwise oriented when viewed from outside the polyhedron.
//   3. The T type must be an arbitrary-precision type that supports division.
//   4. The polyhedron can be degenerate. All the possibilities are listed
//      next.
//
//      point:
//        vertices.size() == 1, triangles.size() = 0
//
//      line segment:
//        vertices.size() == 2, triangles.size() == 0
//
//      convex polygon:
//        vertices.size() >= 3, triangles.size() > 0 and the
//        vertices are coplanar
//
//      convex polyhedron:
//        vertices.size() >= 3, triangles.size() > 0 and the
//        vertices are not coplanar

#include <GTL/Mathematics/Algebra/Vector.h>
#include <array>
#include <cstddef>
#include <vector>

namespace gtl
{
    template <typename T>
    class ConvexMesh3
    {
    public:
        using value_type = T;

        static size_t constexpr CFG_EMPTY = 0x00000000;
        static size_t constexpr CFG_POINT = 0x00000001;
        static size_t constexpr CFG_SEGMENT = 0x00000002;
        static size_t constexpr CFG_POLYGON = 0x00000004;
        static size_t constexpr CFG_POLYHEDRON = 0x00000008;

        using Vertex = Vector3<T>;
        using Triangle = std::array<size_t, 3>;

        ConvexMesh3()
            :
            configuration(CFG_EMPTY),
            vertices{},
            triangles{}
        {
        }

        size_t configuration;
        std::vector<Vertex> vertices;
        std::vector<Triangle> triangles;

    private:
        friend class UnitTestConvexMesh3;
    };
}
