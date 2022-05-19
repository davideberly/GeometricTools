// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.19

#pragma once

// The Polyhedron3 object represents a simple polyhedron. The vertices must
// all be unique and the indices represent triangles, each triangle a triple
// of lookups into the vertex array. The user must ensure the polyhedron
// is closed. The user must also ensure the polyhedron is simple; that is,
// it can have no self-intersections other than what is defined by the
// vertex-triangle graph.
//
// Comparison operators are not provided. The semantics of equal polyhedra are
// complicated and (at the moment) not useful. The vertices of one polyhedron
// can be a permutation of the other polyhedron, but the polyhedra are the
// same geometrically. It is not clear how to implement an efficient
// comparison that does not process all possible cyclic permutations.

#include <GTL/Mathematics/Algebra/Vector.h>
#include <cmath>
#include <cstddef>
#include <map>
#include <set>
#include <vector>

namespace gtl
{
    template <typename T>
    class Polyhedron3
    {
    public:
        using value_type = T;

        // The polyhedron has the specified number of vertices and triangles.
        // The user can set the vertices and indices as needed.
        Polyhedron3(size_t inNumVertices, size_t inNumTriangles, bool inCounterClockwise)
            :
            vertices(inNumVertices),
            indices(3 * inNumTriangles),
            counterClockwise(inCounterClockwise)
        {
            GTL_ARGUMENT_ASSERT(
                vertices.size() >= 4 && indices.size() >= 12,
                "Invalid inputs.");
        }

        // The polyhedron is specified as by a vertex pool and indices into
        // that pool. The indices is an array of triples of integers because
        // the polyhedron faces must be triangles.
        Polyhedron3(std::vector<Vector3<T>> const& inVertexPool,
            std::vector<size_t> const& inIndices, bool inCounterClockwise)
            :
            vertices{},
            indices(inIndices.size()),
            counterClockwise(inCounterClockwise)
        {
            GTL_ARGUMENT_ASSERT(
                inIndices.size() >= 12 && (inIndices.size() % 3) == 0,
                "Invalid inputs.");

            // Get the unique set of used indices.
            std::set<size_t> usedIndices{};
            for (size_t i = 0; i < inIndices.size(); ++i)
            {
                usedIndices.insert(inIndices[i]);
            }

            // Locate the used vertices and pack them into an array.
            vertices.resize(usedIndices.size());
            size_t numVertices = 0;
            std::map<size_t, size_t> vmap{};
            for (auto const& index : usedIndices)
            {
                vertices[numVertices] = inVertexPool[index];
                vmap.insert(std::make_pair(index, numVertices));
                ++numVertices;
            }

            // Reassign the old indices to the new indices.
            for (size_t i = 0; i < inIndices.size(); ++i)
            {
                indices[i] = vmap.find(inIndices[i])->second;
            }
        }

        // Geometric queries. These produce correct results regardless of
        // whether the triangles are in clockwise or counterclockwise order.
        Vector3<T> ComputeVertexAverage() const
        {
            Vector3<T> average{};  // zero vector
            for (auto const& vertex : vertices)
            {
                average += vertex;
            }
            average /= static_cast<T>(vertices.size());
            return average;
        }

        T ComputeSurfaceArea() const
        {
            T surfaceArea = C_<T>(0);
            size_t const numTriangles = indices.size() / 3;
            auto triangles = reinterpret_cast<std::array<size_t, 3> const*>(indices.data());
            for (size_t t = 0; t < numTriangles; ++t)
            {
                size_t v0 = triangles[t][0];
                size_t v1 = triangles[t][1];
                size_t v2 = triangles[t][2];
                Vector3<T> edge0 = vertices[v1] - vertices[v0];
                Vector3<T> edge1 = vertices[v2] - vertices[v0];
                Vector3<T> cross = Cross(edge0, edge1);
                surfaceArea += Length(cross);
            }
            surfaceArea *= C_<T>(1, 2);
            return surfaceArea;
        }

        T ComputeVolume() const
        {
            T volume = C_<T>(0);
            size_t const numTriangles = indices.size() / 3;
            auto triangles = reinterpret_cast<std::array<size_t, 3> const*>(indices.data());
            for (size_t t = 0; t < numTriangles; ++t)
            {
                size_t v0 = triangles[t][0];
                size_t v1 = triangles[t][1];
                size_t v2 = triangles[t][2];
                volume += DotCross(vertices[v0], vertices[v1], vertices[v2]);
            }
            volume /= C_<T>(6);
            return std::fabs(volume);
        }

        std::vector<Vector3<T>> vertices;
        std::vector<size_t> indices;
        bool counterClockwise;

    private:
        friend class UnitTestPolyhedron3;
    };
}
