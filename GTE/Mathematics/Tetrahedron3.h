// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The tetrahedron is represented as an array of four vertices, V[i] for
// 0 <= i <= 3. The vertices are ordered so that the triangular faces are
// counterclockwise-ordered triangles when viewed by an observer outside the
// tetrahedron: face 0 = <V[0],V[2],V[1]>, face 1 = <V[0],V[1],V[3]>,
// face 2 = <V[0],V[3],V[2]> and face 3 = <V[1],V[2],V[3]>. The canonical
// tetrahedron has V[0] = (0,0,0), V[1] = (1,0,0), V[2] = (0,1,0) and
// V[3] = (0,0,1).

#include <Mathematics/Hyperplane.h>
#include <Mathematics/Vector3.h>
#include <array>
#include <cstddef>

namespace gte
{
    template <typename T>
    class Tetrahedron3
    {
    public:
        // Construction and destruction. The default constructor sets the
        // vertices to (0,0,0), (1,0,0), (0,1,0) and (0,0,1).
        Tetrahedron3()
            :
            v{ Vector3<T>::Zero(), Vector3<T>::Unit(0),
               Vector3<T>::Unit(1), Vector3<T>::Unit(2) }
        {
        }

        Tetrahedron3(Vector3<T> const& v0, Vector3<T> const& v1,
            Vector3<T> const& v2, Vector3<T> const& v3)
            :
            v{ v0, v1, v2, v3 }
        {
        }

        Tetrahedron3(std::array<Vector3<T>, 4> const& inV)
            :
            v(inV)
        {
        }

        // Get the vertex indices for the specified face. The input 'face'
        // must be in {0,1,2,3}.
        static inline std::array<size_t, 3> const& GetFaceIndices(size_t face)
        {
            static std::array<std::array<size_t, 3>, 4> const sFaceIndices =
            { {
                { 0, 2, 1 },
                { 0, 1, 3 },
                { 0, 3, 2 },
                { 1, 2, 3 }
            } };
            return sFaceIndices[face];
        }

        static inline std::array<size_t, 12> const& GetAllFaceIndices()
        {
            static std::array<size_t, 12> sAllFaceIndices =
            {
                0, 2, 1,
                0, 1, 3,
                0, 3, 2,
                1, 2, 3
            };
            return sAllFaceIndices;
        }

        // Get the vertex indices for the specified edge. The input 'edge'
        // must be in {0,1,2,3,4,5}.
        static inline std::array<size_t, 2> const& GetEdgeIndices(size_t edge)
        {
            static std::array<std::array<size_t, 2>, 6> sEdgeIndices =
            { {
                { 0, 1 },
                { 0, 2 },
                { 0, 3 },
                { 1, 2 },
                { 1, 3 },
                { 2, 3 }
            } };
            return sEdgeIndices[edge];
        }

        static inline std::array<size_t, 12> const& GetAllEdgeIndices()
        {
            static std::array<size_t, 12> sAllEdgeIndices =
            {
                0, 1, 0, 2, 0, 3, 1, 2, 1, 3, 2, 3
            };
            return sAllEdgeIndices;
        }

        // Get the vertex indices for the edges with the appropriately ordered
        // adjacent indices. The input 'edge' must be in {0,1,2,3,4,5}. The
        // output is {v0,v1,v2,v3} where the edge is {v0,v1}. The triangles
        // sharing the edge are {v0,v2,v1} and {v0,v1,v3}.
        static inline std::array<size_t, 4> const& GetEdgeAugmented(size_t edge)
        {
            static std::array<std::array<size_t, 4>, 6> sEdgeAugmented =
            { {
                { 0, 1, 2, 3 },
                { 0, 2, 3, 1 },
                { 0, 3, 1, 2 },
                { 1, 2, 0, 3 },
                { 1, 3, 2, 0 },
                { 2, 3, 0, 1}
            } };
            return sEdgeAugmented[edge];
        }

        // Get the augmented indices for the vertices with the appropriately
        // ordered adjacent indices. The input 'vertex' must be in {0,1,2,3}.
        // The output is {v0,v1,v2,v3} where the vertex is v0. The triangles
        // sharing the vertex are {v0,v1,v2}, {v0,v2,v3} and {v0,v3,v1}.
        static inline std::array<size_t, 4> const& GetVertexAugmented(size_t vertex)
        {
            static std::array<std::array<size_t, 4>, 4> sVertexAugmented =
            { {
                { 0, 1, 3, 2 },
                { 1, 3, 0, 2 },
                { 2, 1, 0, 3 },
                { 3, 2, 0, 1 },
            } };
            return sVertexAugmented[vertex];
        }

        // Compute a face normal. The input 'face' must be in {0,1,2,3}
        // and correspond to faces {{0,2,1},{0,1,3},{0,3,2},{1,2,3}}.
        Vector3<T> ComputeFaceNormal(size_t face) const
        {
            // Compute the normal for face <v0,v1,v2>.
            auto const& indices = GetFaceIndices(face);
            auto edge10 = v[indices[1]] - v[indices[0]];
            auto edge20 = v[indices[2]] - v[indices[0]];
            auto normal = UnitCross(edge10, edge20);
            return normal;
        }

        // Compute an edge normal, an average of the normals of the 2 faces
        // sharing the edge. The input 'edge' must be in {0,1,2,3,4,5} and
        // correspond to edges {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}}.
        Vector3<T> ComputeEdgeNormal(size_t edge) const
        {
            // Compute the weighted average of normals for faces <v0,a0,v1>
            // and <v0,v1,a1> shared by edge <v0,v1>. In the comments,
            // E10 = V[v1]-V[v0], E20 = V[v2]-V[v0], E30 = V[v3]-V[v0] and
            // E23 = V[i2]-V[i3]. The unnormalized vector is
            //   N = E20 x E10 + E10 x E30
            //     = E20 x E10 - E30 x E10
            //     = (E20 - E30) x E10
            //     = E23 x E10
            auto const& indices = GetEdgeAugmented(edge);
            auto edge23 = v[indices[2]] - v[indices[3]];
            auto edge10 = v[indices[1]] - v[indices[0]];
            auto normal = UnitCross(edge23, edge10);
            return normal;
        }

        // Compute a vertex normal, an average of the normals of the 3 faces
        // sharing the vertex. The input 'vertex' must be in {0,1,2,3} and
        // are the indices into the tetrahedron vertex array. The algebra
        // shows that the vertex normal is the negative normal of the face
        // opposite the vertex.
        Vector3<T> ComputeVertexNormal(size_t vertex) const
        {
            // Compute the weighted average of normals for faces <v0,v1,v2>,
            // <v0,v2,v3> and <v0,v3,v1>. In the comments, E10 = V[v1]-V[v0],
            // E20 = V[v2]-V[v0, E30 = V[v3]-V[v0], E12 = V[v1]-V[v2],
            // E21 = V[v2]-V[v1] and E31 = V[v3]-V[v1]. The unnormalized
            // vector is
            //   N = E10 x E20 + E20 x E30 + E30 x E10
            //     = E10 x E20 - E30 x E20 + E30 x E10 - E10 x E10
            //     = E13 x E20 + E31 x E10
            //     = E13 x E20 - E13 x E10
            //     = E13 x E21
            auto const& indices = GetVertexAugmented(vertex);
            auto edge13 = v[indices[1]] - v[indices[3]];
            auto edge21 = v[indices[2]] - v[indices[1]];
            auto normal = UnitCross(edge13, edge21);
            return normal;
        }

        // Construct the planes of the faces. The planes have outer pointing
        // normal vectors. The plane indexing is the same as the face
        // indexing mentioned previously.
        void GetPlanes(std::array<Plane3<T>, 4>& plane) const
        {
            Vector3<T> edge10 = v[1] - v[0];
            Vector3<T> edge20 = v[2] - v[0];
            Vector3<T> edge30 = v[3] - v[0];
            Vector3<T> edge21 = v[2] - v[1];
            Vector3<T> edge31 = v[3] - v[1];

            plane[0].normal = UnitCross(edge20, edge10);  // <v0,v2,v1>
            plane[1].normal = UnitCross(edge10, edge30);  // <v0,v1,v3>
            plane[2].normal = UnitCross(edge30, edge20);  // <v0,v3,v2>
            plane[3].normal = UnitCross(edge21, edge31);  // <v1,v2,v3>

            T det = Dot(edge10, plane[3].normal);
            if (det < static_cast<T>(0))
            {
                // The normals are inner pointing, reverse their directions.
                for (size_t i = 0; i < 4; ++i)
                {
                    plane[i].normal = -plane[i].normal;
                }
            }

            for (size_t i = 0; i < 4; ++i)
            {
                plane[i].constant = Dot(v[i], plane[i].normal);
            }
        }

        Vector3<T> ComputeCentroid() const
        {
            return (v[0] + v[1] + v[2] + v[3]) * static_cast<T>(0.25);
        }

        // Public member access.
        std::array<Vector3<T>, 4> v;

    public:
        // Comparisons to support sorted containers.
        bool operator==(Tetrahedron3 const& tetrahedron) const
        {
            return v == tetrahedron.v;
        }

        bool operator!=(Tetrahedron3 const& tetrahedron) const
        {
            return v != tetrahedron.v;
        }

        bool operator< (Tetrahedron3 const& tetrahedron) const
        {
            return v < tetrahedron.v;
        }

        bool operator<=(Tetrahedron3 const& tetrahedron) const
        {
            return v <= tetrahedron.v;
        }

        bool operator> (Tetrahedron3 const& tetrahedron) const
        {
            return v > tetrahedron.v;
        }

        bool operator>=(Tetrahedron3 const& tetrahedron) const
        {
            return v >= tetrahedron.v;
        }
    };
}
