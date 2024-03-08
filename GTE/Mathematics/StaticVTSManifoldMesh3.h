// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// StaticVTSManifoldMesh3 represents a vertex-triangle-simplex manifold mesh
// for which tetrahedra (the simplices) are provided as a single batch and no
// mesh modification operations are going to be performed on the mesh. The
// class TSManifoldMesh is dynamic and generally slower to build a mesh. The
// underlying C++ container classes lead to significant memory allocation and
// deallocation costs and are also expensive for find operations. Class
// StaticVTSManifoldMesh3 minimizes the memory management costs. Moreover, it
// allows for multithreading which is useful when the numbers of vertices and
// tetrahedra are large. It is a requirement that the input tetrahdera form a
// manifold mesh with consistently ordered tetrahedra. In most applications,
// this requirement is already satisfied. See the comments for static class
// member 'face' regarding ordering of tetrahedra.

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <thread>
#include <vector>

namespace gte
{
    class StaticVTSManifoldMesh3
    {
    public:
        // Use the maximum size_t to denote an invalid index, effectively
        // representing -1.
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        // The tetrahedron is represented as an array of four vertices, V[i] for
        // 0 <= i <= 3. The vertices are ordered so that the triangular faces are
        // counterclockwise-ordered triangles when viewed by an observer outside the
        // tetrahedron: face 0 = <V[0],V[2],V[1]>, face 1 = <V[0],V[1],V[3]>,
        // face 2 = <V[0],V[3],V[2]> and face 3 = <V[1],V[2],V[3]>. The canonical
        // tetrahedron has V[0] = (0,0,0), V[1] = (1,0,0), V[2] = (0,1,0) and
        // V[3] = (0,0,1).
        static std::array<std::array<size_t, 3>, 4> constexpr face =
        { {
            { 0, 2, 1 },
            { 0, 1, 3 },
            { 0, 3, 2 },
            { 1, 2, 3 }
        } };

        // The class objects are stored as
        //   std::vector<Vertex> vertices(numVertices);
        // If V is a vertex index with 0 <= V < numVertices, then vertices[V]
        // stores information about faces and tetrahedra that are adjacent
        // to V. The member pointers are addresses into a contiguous block of
        // memory in order to minimize the costs of memory management. The
        // block of memory has worst-case allocation of 60 * numTetrahedra
        // elements of type size_t.
        class Vertex
        {
        public:
            Vertex()
                :
                mNumSAdjacents(0),
                mNumVAdjacents(0),
                mVAdjacents(nullptr),
                mNumFAdjacents(0),
                mFAdjacents(nullptr)
            {
            }

            // The members are read-only.
            inline size_t GetNumSAdjacents() const
            {
                return mNumSAdjacents;
            }

            inline size_t GetNumVAdjacents() const
            {
                // The number of adjacent vertices is bounded by three times
                // the number of tetrahedra sharing the vertex.
                return mNumVAdjacents;
            }

            inline size_t const* GetVAdjacents() const
            {
                return mVAdjacents;
            }

            inline size_t GetNumFAdjacents() const
            {
                // The number of adjacent (outgoing) faces is the same as
                // three times the number of tetrahedra sharing the vertex.
                return mNumFAdjacents;
            }

            inline std::array<size_t, 4> const* GetFAdjacents() const
            {
                return mFAdjacents;
            }

        private:
            // Only StaticVTSManifoldMesh3 may write the members of this class.
            friend class StaticVTSManifoldMesh3;

            void Initialize(size_t numSAdjacent, size_t*& storage)
            {
                mNumSAdjacents = numSAdjacent;
                mNumVAdjacents = 0;
                mVAdjacents = storage;
                storage += 3 * mNumSAdjacents;
                mNumFAdjacents = 0;
                mFAdjacents = reinterpret_cast<std::array<size_t, 4>*>(storage);
                storage += 12 * mNumSAdjacents;  // 3 faces per vertex, 4 indices per element
            }

            void InsertVAdjacent(size_t v)
            {
                auto* vAdjacents = mVAdjacents;
                for (size_t i = 0; i < mNumVAdjacents; ++i, ++vAdjacents)
                {
                    if (v == *vAdjacents)
                    {
                        // The vertex v is already in the adjacents list.
                        return;
                    }
                }

                // The vertex v is not in the adjacents list, so append it.
                *vAdjacents = v;
                ++mNumVAdjacents;
            }

            void InsertFAdjacent(size_t v1, size_t v2, size_t s)
            {
                mFAdjacents[mNumFAdjacents++] = { v1, v2, s, invalid };
            }

            size_t mNumSAdjacents;
            size_t mNumVAdjacents;   // <= 3 * mNumSAdjacents
            size_t* mVAdjacents;    // [3 * mNumSAdjacents]
            size_t mNumFAdjacents;   // <= 3 * mNumSAdjacents after mesh construction
            std::array<size_t, 4>* mFAdjacents; // [3 * mNumSAdjacents], <AV0,AV1,LS,RS>
        };

        // Preconditions.
        //   1. The tetrahedra input must have size 1 or larger.
        //   2. The number of vertices must be 4 or larger.
        //   3. The tetrahedra must form a manifold mesh.
        //   4. Each tetrahedra must be nondegenerate; no repeated vertices.
        //   5. The tetrahedra must all be ordered counterclockwise or all
        //      ordered clockwise; no mixed chirality.
        // Set numThreads to 2 or larger to activate multithreading in the
        // mesh construction. If numThreads is 0 or 1, the construction
        // occurs in the main thread.
        StaticVTSManifoldMesh3(
            size_t numVertices,
            std::vector<std::array<size_t, 4>> const& tetrahedra,
            size_t numThreads)
            :
            mVertices(numVertices),
            mStorage(60 * tetrahedra.size(), invalid),
            mTetrahedra(tetrahedra),
            mAdjacents(tetrahedra.size(), { invalid, invalid, invalid, invalid }),
            mMinTetrahedraAtVertex(0),
            mMaxTetrahedraAtVertex(0)
        {
            LogAssert(numVertices >= 4 && tetrahedra.size() > 0, "invalid input");

            std::vector<size_t> numTetrahedraAtVertex(numVertices, 0);
            GetNumTetrahedraAtVertex(numTetrahedraAtVertex);
            InitializeVertexStorage(numTetrahedraAtVertex);
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

        // Determine whether or not the unordered face <v0,v1,v2> exists.
        bool FaceExists(size_t v0, size_t v1, size_t v2) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v2 < mVertices.size() &&
                v0 != v1 && v0 != v2 && v1 != v2)
            {
                // Sort the face to <u0,u1,u2> where u0 = min(u0,u1,u2)
                // and the face is CCW when viewed from outside the
                // tetrahedron.
                size_t u0{}, u1{}, u2{};
                SortFace(v0, v1, v2, u0, u1, u2);

                auto* face0 = GetOutgoingFace(u0, u1, u2);
                if (face0 != nullptr)
                {
                    return true;
                }

                // An outgoing face was not found. Try to find an incoming
                // face.
                face0 = GetOutgoingFace(u0, u2, u1);
                if (face0 != nullptr)
                {
                    return true;
                }
            }
            return false;
        }

        // Get the adjacent tetrahedra for the unordered face <v0,v1,v2>. The
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
        //   4. The unordered face <v0,v1,v2> does not exist.
        //
        // It is possible to distinguish between the 4 cases by examining the
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
                // Sort the face to <u0,u1,u2> where u0 = min(u0,u1,u2)
                // and the face is CCW when viewed from outside the
                // tetrahedron.
                size_t u0{}, u1{}, u2{};
                SortFace(v0, v1, v2, u0, u1, u2);

                auto* face0 = GetOutgoingFace(u0, u1, u2);
                if (face0 != nullptr)
                {
                    adj0 = (*face0)[2];
                    adj1 = (*face0)[3];
                    return true;
                }

                face0 = GetOutgoingFace(u0, u2, u1);
                if (face0 != nullptr)
                {
                    adj0 = (*face0)[2];
                    adj1 = (*face0)[3];
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
        // increment occurs 4 times per tetrahedra.
        void GetNumTetrahedraAtVertex(std::vector<size_t>& counts)
        {
            for (auto const& tri : mTetrahedra)
            {
                for (size_t i = 0; i < 4; ++i)
                {
                    ++counts[tri[i]];
                }
            }

            auto extremes = std::minmax_element(counts.begin(), counts.end());
            mMinTetrahedraAtVertex = *extremes.first;
            mMaxTetrahedraAtVertex = *extremes.second;
        }

        // Assign the storage subblocks to the vertices. The mNumVAdjacents
        // member is incremented later during a tetrahedron traversal and is
        // used as an index into mVAdjacents during the traversal.
        void InitializeVertexStorage(std::vector<size_t> const& numTetrahedraAtVertex)
        {
            auto* storage = mStorage.data();
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                mVertices[v].Initialize(numTetrahedraAtVertex[v], storage);
            }
        }

        // Populate each vertex with its adjacent L-tetrahedra and adjacent
        // faces.
        void UpdateVertexAdjacents(size_t t)
        {
            // Iterate over all vertex pairs (u0,u1). Update u0's vertex
            // adjacents with u1 and update u1's vertex adjacents with u0.
            auto const& tetra = mTetrahedra[t];
            size_t v0 = tetra[0];
            size_t v1 = tetra[1];
            size_t v2 = tetra[2];
            size_t v3 = tetra[3];
            auto& vertex0 = mVertices[v0];
            auto& vertex1 = mVertices[v1];
            auto& vertex2 = mVertices[v2];
            auto& vertex3 = mVertices[v3];
            vertex0.InsertVAdjacent(v1);
            vertex0.InsertVAdjacent(v2);
            vertex0.InsertVAdjacent(v3);
            vertex1.InsertVAdjacent(v2);
            vertex1.InsertVAdjacent(v0);
            vertex1.InsertVAdjacent(v3);
            vertex2.InsertVAdjacent(v0);
            vertex2.InsertVAdjacent(v1);
            vertex2.InsertVAdjacent(v3);
            vertex3.InsertVAdjacent(v1);
            vertex3.InsertVAdjacent(v0);
            vertex3.InsertVAdjacent(v2);
        }

        void UpdateFaceAdjacents(size_t t)
        {
            auto const& tetra = mTetrahedra[t];
            for (size_t i = 0; i < 4; ++i)
            {
                // Get an outgoing face <v0,v1,v2>, which is CCW when
                // viewed from outside the tetrahedron.
                size_t v0 = tetra[face[i][0]];
                size_t v1 = tetra[face[i][1]];
                size_t v2 = tetra[face[i][2]];

                // Sort the face to <u0,u1,u2> where u0 = min(u0,u1,u2)
                // and the face is CCW when viewed from outside the
                // tetrahedron.
                size_t u0{}, u1{}, u2{};
                SortFace(v0, v1, v2, u0, u1, u2);

                // Update the face adjacency information at u0.
                mVertices[u0].InsertFAdjacent(u1, u2, t);
            }
        }

        void PopulateVertices()
        {
            for (size_t t = 0; t < mTetrahedra.size(); ++t)
            {
                UpdateVertexAdjacents(t);
                UpdateFaceAdjacents(t);
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
            for (size_t t = 0; t < mTetrahedra.size(); ++t)
            {
                UpdateAdjacencyForTetrahedron(t);
            }
        }

        void UpdateAdjacencyForSharedFacesMultithreaded(size_t numThreads)
        {
            size_t const numTetrahedra = mTetrahedra.size();
            size_t const numTetrahedraPerThread = numTetrahedra / numThreads;
            std::vector<size_t> tmin(numThreads), tsup(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                tmin[i] = i * numTetrahedraPerThread;
                tsup[i] = (i + 1) * numTetrahedraPerThread;
            }
            tsup.back() = numTetrahedra;

            std::vector<std::thread> process(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i] = std::thread([this, i, &tmin, &tsup]()
                {
                    for (size_t t = tmin[i]; t < tsup[i]; ++t)
                    {
                        UpdateAdjacencyForTetrahedron(t);
                    }
                });
            }

            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i].join();
            }
        }

        void UpdateAdjacencyForTetrahedron(size_t t)
        {
            auto const& tetra = mTetrahedra[t];
            for (size_t i = 0; i < 4; ++i)
            {
                // Get an outgoing face <v0,v1,v2>, which is CCW when
                // viewed from outside the tetrahedron.
                size_t v0 = tetra[face[i][0]];
                size_t v1 = tetra[face[i][1]];
                size_t v2 = tetra[face[i][2]];

                // Sort the face to <u0,u1,u2> where u0 = min(u0,u1,u2)
                // and the face is CCW when viewed from outside the
                // tetrahedron.
                size_t u0{}, u1{}, u2{};
                SortFace(v0, v1, v2, u0, u1, u2);

                // The outgoing face from v0 is <v0,v1,v2> and has adjacency
                // quad <v1,v2,LT0,invalid>. If <v0,v2,v1> is an outgoing face
                // from v0 with adjacency quad <v2,v1,LT1,invalid>, update the
                // v0 adjacent quad to <v1,v2,LT0,LT1>; that is, RT0 = LT1.
                // Although it is possible at this time to update the v0
                // adjacent quad to <v2,v1,LT1,LT0>, where RT1 = LT0, this
                // quad will be processed when the outgoing face is visited at
                // another time. By not updating <v2,v1,LT1,invalid> now, two
                // writes are avoided to each of RT0 and RT1. This also
                // supports the multithreaded approach because one thread
                // never has the potential to write to the same memory
                // location that another thread writes to.
                auto* face0 = GetOutgoingFace(u0, u1, u2);
                auto* face1 = GetOutgoingFace(u0, u2, u1);
                if (face0 != nullptr && face1 != nullptr)
                {
                    (*face0)[3] = (*face1)[2];  // RT0 = LT1
                    mAdjacents[t][i] = (*face1)[2];
                }
            }
        }

        std::array<size_t, 4>* GetOutgoingFace(size_t w0, size_t w1, size_t w2) const
        {
            auto& vertex = mVertices[w0];
            size_t const numFAdjacents = vertex.mNumFAdjacents;
            auto* fAdjacents = vertex.mFAdjacents;
            for (size_t j = 0; j < numFAdjacents; ++j, ++fAdjacents)
            {
                if ((*fAdjacents)[0] == w1 && (*fAdjacents)[1] == w2)
                {
                    return fAdjacents;
                }
            }
            return nullptr;
        }

        static void SortFace(size_t v0, size_t v1, size_t v2,
            size_t& u0, size_t& u1, size_t& u2)
        {
            if (v0 < v1)
            {
                if (v0 < v2)
                {
                    // v0 is minimum
                    u0 = v0;
                    u1 = v1;
                    u2 = v2;
                }
                else
                {
                    // v2 is minimum
                    u0 = v2;
                    u1 = v0;
                    u2 = v1;
                }
            }
            else
            {
                if (v1 < v2)
                {
                    // v1 is minimum
                    u0 = v1;
                    u1 = v2;
                    u2 = v0;
                }
                else
                {
                    // v2 is minimum
                    u0 = v2;
                    u1 = v0;
                    u2 = v1;
                }
            }
        }

        std::vector<Vertex> mVertices;
        std::vector<size_t> mStorage;
        std::vector<std::array<size_t, 4>> mTetrahedra;
        std::vector<std::array<size_t, 4>> mAdjacents;
        size_t mMinTetrahedraAtVertex;
        size_t mMaxTetrahedraAtVertex;
    };
}
