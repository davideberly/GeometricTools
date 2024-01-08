// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// MeshStaticManifold3 represents a vertex-face-tetrahedron manifold mesh
// for which tetrahedra (the simplices) are provided as a single batch and no
// mesh modification operations are going to be performed on the mesh.
// MeshStaticManifold3 significantly outperforms VTSManifoldMesh.
// VTSManifoldMesh is dynamic, allowing tetrahedron insertions and removals at
// any time. The underlying C++ container classes lead to significant memory
// allocation and deallocation costs and are also expensive for find
// operations. MeshStaticManifold3 minimizes the memory management costs.
// Moreover, it allows for multithreading which is useful when the numbers of
// vertices and tetrahedra are large. It is a requirement that the input
// tetrahedra form a manifold mesh with consistently ordered tetrahedra. In
// most applications, this requirement is already satisfied.

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <thread>
#include <vector>

namespace gte
{
    class MeshStaticManifold3
    {
    public:
        // Use the maximum size_t to denote an invalid index, effectively
        // representing -1.
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        // The tetrahedron is represented as an array of four vertices, V[i]
        // for 0 <= i <= 3. The vertices are ordered so that the triangle
        // faces are counterclockwise ordered when viewed by an observer
        // outside the tetrahedron: face[0] = <V[1],V[2],V[3]>,
        // face[1] = <V[0],V[3],V[2]>, face[2] = <V[0],V[1],V[3]> and
        // face[3] = <V[0],V[2],V[1]>. Observe that for face[i], the vertex
        // opposite the face is V[i]. The canonical tetrahedron has
        // V[0] = (0,0,0), V[1] = (1,0,0), V[2] = (0,1,0) and V[3] = (0,0,1).
        static std::array<std::array<size_t, 3>, 4> constexpr face =
        { {
            { 1, 2, 3 }, { 0, 3, 2 }, { 0, 1, 3 }, { 0, 2, 1 }
        } };

        // The vertices are stored as std::vector<Vertex> vertex(numVertices).
        // If tetrahedron[t0] = <v0,v1,v2,v3>, then vertex[v0] contains a
        // 5-tuple {v1,v2,v3,t0,a0}. The unordered face (v1,v2,v3) is opposite
        // v0. If there is no adjacent tetrahedron sharing (v1,v2,v3), then
        // a0 is invalid. If there is an adjacent tetrahedron, then a0 is the
        // index for that tetrahedron. Let tetrahedron[a0] = <v1,v3,v2,v4>;
        // then vertex[v4] contains a 5-tuple {v1,v3,v2,a0,t0}.
        // 
        // The member pointers of Vertex are addresses into a contiguous block
        // of memory in order to minimize the costs of memory management. The
        // block of memory has 20 * numTetrahedra elements of type size_t.
        class Vertex
        {
        public:
            Vertex()
                :
                mNumAdjacents(0),
                mAdjacents(nullptr)
            {
            }

            // The members are read-only.
            inline size_t GetNumAdjacents() const
            {
                return mNumAdjacents;
            }

            inline std::array<size_t, 5> const* GetAdjacents() const
            {
                return mAdjacents;
            }

        private:
            // Only MeshStaticManifold3 may write the members of this class.
            friend class MeshStaticManifold3;

            void Initialize(size_t numAdjacents, size_t*& storage)
            {
                mNumAdjacents = 0;
                mAdjacents = reinterpret_cast<std::array<size_t, 5>*>(storage);
                storage += 5 * numAdjacents;
            }

            void Insert(size_t v1, size_t v2, size_t v3, size_t s, size_t location)
            {
                mAdjacents[mNumAdjacents++] = { v1, v2, v3, s, location };
            }

            // The number of tetrahedra sharing v0. The value starts at zero
            // and is incremented during the MeshStaticManifold3::Populate
            // execution.
            size_t mNumAdjacents;

            // If tetrahedron t0 is <v0,v1,v2,v3> in counterclockwise order,
            // then the corresponding adjacents element is {v1,v2,v3,t0,a0},
            // where a0 is invalid when <v1,v2,v3> is contained by a single
            // tetrahedron or a0 is the index for the adjacent tetrahedron
            // when <v1,v2,v3> is contained by two tetrahedra.
            std::array<size_t, 5>* mAdjacents;
        };

        // Preconditions.
        //   1. The tetrahedra input must have size 1 or larger.
        //   2. The number of vertices must be 4 or larger.
        //   3. The tetrahedra must form a manifold mesh.
        //   4. Each tetrahedron must be nondegenerate; no repeated vertices.
        //   5. The tetrahedra must all be ordered counterclockwise.
        // Set numThreads to 2 or larger to activate multithreading in the
        // mesh construction. If numThreads is 0 or 1, the construction
        // occurs in the main thread.
        MeshStaticManifold3(
            size_t numVertices,
            std::vector<std::array<size_t, 4>> const& tetrahedra,
            size_t numThreads)
            :
            mVertices(numVertices),
            mStorage(20 * tetrahedra.size(), invalid),
            mTetrahedra(tetrahedra),
            mAdjacents(tetrahedra.size(), { invalid, invalid, invalid, invalid }),
            mMinTetrahedraAtVertex(0),
            mMaxTetrahedraAtVertex(0)
        {
            LogAssert(numVertices >= 4 && tetrahedra.size() > 0, "Invalid input.");

            std::vector<size_t> numTetrahedraAtVertex(numVertices, 0);
            GetNumTetrahedraAtVertex(numTetrahedraAtVertex);
            InitializeStorage(numTetrahedraAtVertex);
            PopulateVertices();
            UpdateAdjacencyForSharedFaces(numThreads);
        }

        // Member access.
        inline std::vector<Vertex> const& GetVertices() const
        {
            return mVertices;
        }

        // Each 4-tuple contains indices into the vertices.
        inline std::vector<std::array<size_t, 4>> const& GetTetrahedra() const
        {
            return mTetrahedra;
        }

        // Each 4-tuple contains indices into the tetrahedra.
        inline std::vector<std::array<size_t, 4>> const& GetAdjacents() const
        {
            return mAdjacents;
        }

        inline size_t GetMinNumTetrahedraAtVertex() const
        {
            return mMinTetrahedraAtVertex;
        }

        inline size_t GetMaxNumTetrahedraAtVertex() const
        {
            return mMaxTetrahedraAtVertex;
        }

        // Determine whether or not the unordered face (v0,v1,v2) exists.
        bool FaceExists(size_t v0, size_t v1, size_t v2) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v2 < mVertices.size() &&
                v0 != v1 && v0 != v2 && v1 != v2)
            {
                return GetOrderedFace(v0, v1, v2) != nullptr
                    || GetOrderedFace(v0, v2, v1) != nullptr;
            }
            return false;
        }

        // Get the adjacent tetrahedra for the unordered face (v0,v1,v2). The
        // returned adjacent tetrahedron indices adj0 and adj1 are the
        // following:
        //
        //   1. <v0,v1,v2> and <v0,v2,v1> are both outgoing edges, so the edge
        //      is shared by two tetrahedra and both adj0 and adj1 are valid
        //      (not equal to 'invalid'). The index adj0 is the L-tetrahedron
        //      for <v0,v1,v2> and the index adj1 is the R-tetrahedron for
        //      <v0,v1,v2>. Equivalently, adj0 is the R-tetrahedron for
        //      <v0,v2,v1> and adj1 is the L-tetrahedron for <v0,v2,v1>.
        // 
        //   2. <v0,v1,v2> is outgoing but <v0,v2,v1> is not outgoing. The
        //      index adj0 is the L-tetrahedron for <v0,v1,v2> and the index
        //      adj1 is 'invalid' (no R-tetrahedron).
        // 
        //   3. <v0,v2,v1> is outgoing but <v0,v1,v2> is not outgoing. The
        //      index adj0 is 'invalid' (no L-tetrahedron) for <v0,v2,v1> and
        //      the index adj1 is the R-tetrahedron for <v0,v2,v1>.
        // 
        //   4. Neither <v0,v1,v2> nor <v0,v2,v1> exist; that is, the face
        //      does not occur for any tetrahedron.
        //
        // It is possible to distinguish among the 4 cases by examining the
        // returned indices:
        //   (1) returns (valid, valid) and Boolean 'true'
        //   (2) returns (valid, invalid) and Boolean 'true'
        //   (3) returns (invalid, valid) and Boolean 'true'
        //   (4) returns (invalid, invalid) and Boolean 'false'
        bool GetAdjacentTetrahedra(size_t v0, size_t v1, size_t v2, size_t& adj0, size_t& adj1) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v2 < mVertices.size() &&
                v0 != v1 && v0 != v2 && v1 != v2)
            {
                auto* adjacents0 = GetOrderedFace(v0, v1, v2);
                if (adjacents0 != nullptr)
                {
                    adj0 = (*adjacents0)[2];
                    adj1 = (*adjacents0)[3];
                    return true;
                }

                auto* adjacents1 = GetOrderedFace(v0, v2, v1);
                if (adjacents1 != nullptr)
                {
                    adj0 = (*adjacents1)[2];
                    adj1 = (*adjacents1)[3];
                    return true;
                }
            }

            adj0 = invalid;
            adj1 = invalid;
            return false;
        }

    protected:
        // Count the number of tetrahedra sharing each vertex. The total
        // number of indices for tetrahedra adjacent to vertices is
        // 4 * numTetrahedra. This is easy to see from the code where an
        // increment occurs 4 times per tetrahedra. Each adjacent element has
        // 5 components of type size_t, so the adjacent storage requires
        // 20 * numTetrahedra values of type size_t.
        void GetNumTetrahedraAtVertex(std::vector<size_t>& counts)
        {
            for (auto const& tri : mTetrahedra)
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    ++counts[tri[i]];
                }
            }

            // The minimum and maximum tetrahedra counts are for statistical
            // information.
            auto extremes = std::minmax_element(counts.begin(), counts.end());
            mMinTetrahedraAtVertex = *extremes.first;
            mMaxTetrahedraAtVertex = *extremes.second;
        }

        // Assign the storage subblocks to the vertices.
        void InitializeStorage(std::vector<size_t> const& numTetrahedraAtVertex)
        {
            auto* storage = mStorage.data();
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                mVertices[v].Initialize(numTetrahedraAtVertex[v], storage);
            }
        }

        // Populate the adjacency information for the vertices. The number of
        // vertex[v] adjacents is 3 * numTetrahedraAtVertex[v]. This requires
        // 20 * numTetrahedraAtVertex[v] elements of type size_t. For the
        // entire mesh, we need 20 * numTetrahedra elements of type size_t.
        void PopulateVertices()
        {
            for (size_t t = 0; t < mTetrahedra.size(); ++t)
            {
                auto const& tet = mTetrahedra[t];
                size_t v0 = tet[0];
                size_t v1 = tet[1];
                size_t v2 = tet[2];
                size_t v3 = tet[3];

                // The last arguments (i = 0, 1, 2 or 3) are used to set the
                // correct mAdjacents[][i] indices. These arguments are
                // replaced later by the actual indices for adjacent
                // tetrahedra sharing the face.
                mVertices[v0].Insert(v1, v2, v3, t, 0);
                mVertices[v1].Insert(v0, v3, v2, t, 1);
                mVertices[v2].Insert(v0, v1, v3, t, 2);
                mVertices[v3].Insert(v0, v2, v1, t, 3);
            }
        }

        // Update tetrahedra adjacency information for faces that are shared
        // by two tetrahedra.
        void UpdateAdjacencyForSharedFaces(size_t numThreads)
        {
            if (numThreads <= 1)
            {
                UpdateAdjacencyForSharedFacesSingleThreaded();
            }
            else
            {
                UpdateAdjacencyForSharedFacesMultithreaded(numThreads);
            }
        }

        void UpdateAdjacencyForSharedFacesSingleThreaded()
        {
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                UpdateAdjacencyForFace(v);
            }
        }

        void UpdateAdjacencyForSharedFacesMultithreaded(size_t numThreads)
        {
            size_t const numVertices = mVertices.size();
            size_t const numVerticesPerThread = numVertices / numThreads;
            std::vector<size_t> vmin(numThreads), vsup(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                vmin[i] = i * numVerticesPerThread;
                vsup[i] = (i + 1) * numVerticesPerThread;
            }
            vsup.back() = numVertices;

            std::vector<std::thread> process(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i] = std::thread([this, i, &vmin, &vsup]()
                {
                    for (size_t v = vmin[i]; v < vsup[i]; ++v)
                    {
                        UpdateAdjacencyForFace(v);
                    }
                });
            }

            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i].join();
            }
        }

        void UpdateAdjacencyForFace(size_t v0)
        {
            auto& vertex0 = mVertices[v0];
            auto* adjacents0 = vertex0.mAdjacents;
            for (size_t j = 0; j < vertex0.mNumAdjacents; ++j, ++adjacents0)
            {
                size_t v1 = (*adjacents0)[0];
                size_t v2 = (*adjacents0)[1];
                size_t v3 = (*adjacents0)[2];

                // The face opposite vertex v0 is (v1,v2,v3). We know that
                // vertex[v0] contains a 5-tuple {v1,v2,v3,tri0,loc0}.
                // Determine whether vertex[v1] contains a 5-tuple
                // {v1,v0,v2,adj1,loc1}.
                auto* adjacents1 = GetOrderedFace(v1, v3, v2);
                if (adjacents1)
                {
                    // The face <v0,v1,v2> has a tetrahedron adjacent to
                    // tetrahedron tet0. Update the vertex adjacency
                    // information for tetrahedron tet0 at that edge.
                    // Tetrahedron a1 adjacency is not updated. It will be
                    // updated when <v1,v0,v2> is visited at another time.
                    // This avoids two writes of the adjacent tetrahedron
                    // indices. It also supports the multithreaded approach
                    // because one thread never has the potential to write to
                    // the same memory location that another thread writes to.
                    size_t tet0 = (*adjacents0)[3];
                    size_t loc0 = (*adjacents0)[4];
                    size_t adj1 = (*adjacents1)[3];
                    (*adjacents0)[4] = adj1;
                    mAdjacents[tet0][loc0] = adj1;
                }
                else
                {
                    // Replace the mAdjacents[] location value (0, 1, 2 or 3)
                    // by an invalid index because face <v1,v0,v2> does not
                    // exist, in which case there is no adjacent tetrahedron
                    // to face <v0,v1,v2>.
                    (*adjacents0)[4] = invalid;
                }
            }
        }

        std::array<size_t, 5>* GetOrderedFace(size_t v0, size_t v1, size_t v2) const
        {
            auto& vertex0 = mVertices[v0];
            auto* adjacents0 = vertex0.mAdjacents;
            for (size_t j = 0; j < vertex0.mNumAdjacents; ++j, ++adjacents0)
            {
                std::array<size_t, 4> tetra =
                {
                    v0,
                    (*adjacents0)[0],
                    (*adjacents0)[1],
                    (*adjacents0)[2]
                };

                std::array<size_t, 3> inFace = { v0, v1 ,v2 };

                for (size_t f = 0; f < 4; ++f)
                {
                    std::array<size_t, 3> compareFace =
                    {
                        tetra[face[f][0]],
                        tetra[face[f][1]],
                        tetra[face[f][2]]
                    };

                    if (inFace == compareFace)
                    {
                        return adjacents0;
                    }
                }
            }
            return nullptr;
        }

        std::vector<Vertex> mVertices;
        std::vector<size_t> mStorage;
        std::vector<std::array<size_t, 4>> mTetrahedra;
        std::vector<std::array<size_t, 4>> mAdjacents;
        size_t mMinTetrahedraAtVertex;
        size_t mMaxTetrahedraAtVertex;
    };
}
