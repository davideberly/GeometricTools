// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <thread>
#include <utility>
#include <vector>

namespace gte
{
    class StaticVETManifoldMesh2
    {
    public:
        // Use the maximum size_t to denote an invalid index, effectively
        // representing -1.
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        // The class objects are stored as
        //   std::vector<Vertex> vertices(numVertices);
        // If V is a vertex index with 0 <= V < numVertices, then vertices[V]
        // stores information about edges and triangles that are adjacent
        // to V. The member pointers are addresses into a contiguous block of
        // memory in order to minimize the costs of memory management. The
        // block of memory has worst-case allocation of 15 * numTriangles
        // elements of type size_t.
        class Vertex
        {
        public:
            Vertex()
                :
                mNumTAdjacents(0),
                mNumVAdjacents(0),
                mVAdjacents(nullptr),
                mNumEAdjacents(0),
                mEAdjacents(nullptr)
            {
            }

            // The members are read-only.
            inline size_t GetNumTAdjacents() const
            {
                return mNumTAdjacents;
            }

            inline size_t GetNumVAdjacents() const
            {
                // The number of adjacent vertices is bounded by twice the
                // number of triangles sharing the vertex.
                return mNumVAdjacents;
            }

            inline size_t const* GetVAdjacents() const
            {
                return mVAdjacents;
            }

            inline size_t GetNumEAdjacents() const
            {
                // The number of adjacent (outgoing) edges is the same as the
                // number of triangles sharing the vertex.
                return mNumEAdjacents;
            }

            inline std::array<size_t, 3> const* GetEAdjacents() const
            {
                return mEAdjacents;
            }

        private:
            // Only StaticVETManifoldMesh2 may write the members of this class.
            friend class StaticVETManifoldMesh2;

            void Initialize(size_t numTAdjacent, size_t*& storage)
            {
                mNumTAdjacents = numTAdjacent;
                mNumVAdjacents = 0;
                mVAdjacents = storage;
                storage += 2 * mNumTAdjacents;
                mNumEAdjacents = 0;
                mEAdjacents = reinterpret_cast<std::array<size_t, 3>*>(storage);
                storage += 3 * mNumTAdjacents;
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

            void InsertEAdjacent(size_t v, size_t t)
            {
                mEAdjacents[mNumEAdjacents++] = { v, t, invalid };
            }

            size_t mNumTAdjacents;
            size_t mNumVAdjacents;   // <= 2 * mNumTAdjacents
            size_t* mVAdjacents;    // [2 * mNumTAdjacents]
            size_t mNumEAdjacents;   // = mNumTAdjacents after mesh construction
            std::array<size_t, 3>* mEAdjacents; // [mNumTAdjacents], <AV,LT,RT>
        };

        // Preconditions.
        //   1. The triangles input must have size 1 or larger.
        //   2. The number of vertices must be 3 or larger.
        //   3. The triangles must form a manifold mesh.
        //   4. Each triangle must be nondegenerate; no repeated vertices.
        //   5. The triangles must all be ordered counterclockwise or all
        //      ordered clockwise; no mixed chirality.
        // Set numThreads to 2 or larger to activate multithreading in the
        // mesh construction. If numThreads is 0 or 1, the construction
        // occurs in the main thread.
        StaticVETManifoldMesh2(
            size_t numVertices,
            std::vector<std::array<size_t, 3>> const& triangles,
            size_t numThreads)
            :
            mVertices(numVertices),
            mStorage(15 * triangles.size(), invalid),
            mTriangles(triangles),
            mAdjacents(triangles.size(), { invalid, invalid, invalid }),
            mMinTrianglesAtVertex(0),
            mMaxTrianglesAtVertex(0)
        {
            LogAssert(numVertices >= 3 && triangles.size() > 0, "invalid input");

            std::vector<size_t> numTrianglesAtVertex(numVertices, 0);
            GetNumTrianglesAtVertex(numTrianglesAtVertex);
            InitializeVertexStorage(numTrianglesAtVertex);
            PopulateVertices();
            UpdateAdjacencyForSharedEdges(numThreads);
        }

        // Member access.
        inline std::vector<Vertex> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<std::array<size_t, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        inline std::vector<std::array<size_t, 3>> const& GetAdjacents() const
        {
            return mAdjacents;
        }

        inline size_t GetMinNumTrianglesAtVertex() const
        {
            return mMinTrianglesAtVertex;
        }

        inline size_t GetMaxNumTrianglesAtVertex() const
        {
            return mMaxTrianglesAtVertex;
        }

        // Determine whether or not the undirected edge (v0,v1) exists.
        bool EdgeExists(size_t v0, size_t v1) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v0 != v1)
            {
                auto const& vertex0 = mVertices[v0];
                for (size_t j = 0; j < vertex0.mNumEAdjacents; ++j)
                {
                    if (v1 == vertex0.mEAdjacents[j][0])
                    {
                        return true;
                    }
                }

                auto const& vertex1 = mVertices[v1];
                for (size_t j = 0; j < vertex1.mNumEAdjacents; ++j)
                {
                    if (v0 == vertex1.mEAdjacents[j][0])
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        // Get the adjacent triangles for the undirected edge (v0,v1). The
        // returned adjacent triangle indices adj0 and adj1 are the following:
        //
        //   1. <v0,v1> and <v1,v0> are both outgoing edges, so the edge is
        //      shared by two triangles and both adj0 and adj1 are valid (not
        //      equal to 'invalid'). The index adj0 is the L-triangle for
        //      <v0,v1> and the index adj1 is the R-triangle for <v0,v1>.
        //      Equivalently, adj0 is the R-triangle for <v1,v0> and adj1 is
        //      the L-triangle for <v1,v0>.
        // 
        //   2. <v0,v1> is outgoing but <v1,v0> is not outgoing. The index
        //      adj0 is the L-triangle for <v0,v1> and the index adj1 is
        //      'invalid' (no R-triangle).
        // 
        //   3. <v1,v0> is outgoing but <v0,v1> is not outgoing. The index
        //      adj0 is 'invalid' (no L-triangle) for <v1,v0> and the index
        //      adj1 is the R-triangle for <v1,v0>.
        // 
        //   4. The outgoing edge <v0,v1> does not exist.
        //
        // It is possible to distinguish between the 3 cases by examining the
        // returned indices:
        //   (1) returns (valid, valid) and Boolean 'true'
        //   (2) returns (valid, invalid) and Boolean 'true'
        //   (3) returns (invalid, valid) and Boolean 'true'
        //   (4) returns (invalid, invalid) and Boolean 'false'
        bool GetAdjacentTriangles(size_t v0, size_t v1, size_t& adj0, size_t& adj1) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v0 != v1)
            {
                auto const& vertex0 = mVertices[v0];
                for (size_t j = 0; j < vertex0.mNumEAdjacents; ++j)
                {
                    if (v1 == vertex0.mEAdjacents[j][0])
                    {
                        adj0 = vertex0.mEAdjacents[j][1];
                        adj1 = vertex0.mEAdjacents[j][2];
                        return true;
                    }
                }

                auto const& vertex1 = mVertices[v1];
                for (size_t j = 0; j < vertex1.mNumEAdjacents; ++j)
                {
                    if (v0 == vertex1.mEAdjacents[j][0])
                    {
                        adj0 = vertex1.mEAdjacents[j][1];
                        adj1 = vertex1.mEAdjacents[j][2];
                        return true;
                    }
                }
            }

            adj0 = invalid;
            adj1 = invalid;
            return false;
        }

        // The connected components are stored in a single std::vector. The
        // range[] std::vector contains indices into components[] for first
        // and last index of each component. The number of components is
        // range.size(). The c-th component has triangle indices stored in
        // components[range[c][0]] through components[range[c][1]] - 1. A loop
        // over the component c uses
        //   for (i = range[c][0]; i < range[c][1]; ++i)
        //   {
        //       <process component[i]>;
        //   }
        void GetComponents(std::vector<std::vector<size_t>>& components) const
        {
            components.clear();

            // The values are 0 (unvisited), 1 (discovered), 2 (finished).
            std::vector<uint32_t> visited(mTriangles.size(), 0);

            // Share a stack for the depth-first search. This avoids
            // allocating and deallocating a stack for each call to
            // DepthFirstSearch.
            std::vector<size_t> sharedStack(mTriangles.size());

            // The code reserves maximum space for the component in order to
            // avoid allocation/deallocation costs associated with resizing
            // caused by push_back.
            std::vector<size_t> sharedComponents(mTriangles.size());

            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                if (visited[t] == 0)
                {
                    size_t numInserted = DepthFirstSearch(t, visited,
                        sharedStack, sharedComponents.data());

                    std::vector<size_t> component(numInserted);
                    std::copy(
                        sharedComponents.begin(),
                        sharedComponents.begin() + numInserted,
                        component.begin());

                    components.push_back(std::move(component));
                }
            }
        }

        // Compute the boundary-edge components of the mesh. These are
        // polygons that are simple for the strict definition of manifold
        // mesh that disallows bow-tie configurations. The GTE mesh
        // implementations do allow bow-tie configurations, in which case
        // some polygons might not be simple. If you select duplicateEndpoints
        // to be false, a component has consecutive vertices
        // (v[0], v[1], ..., v[n-1]) and the polygon has edges
        // (v[0],v[1]), (v[1],v[2]), ..., (v[n-2],v[n-1]), (v[n-1],v[0]).
        // If duplicateEndpoints is set to true, a component has consecutive
        // vertices (v[0], v[1], ..., v[n-1], v[0]), emphasizing that the
        // component is closed.
        void GetBoundaryPolygons(std::vector<std::vector<size_t>>& polygons,
            bool duplicateEndpoints) const
        {
            polygons.clear();

            // Get the boundary edges.
            BoundaryEdgeMap boundaryEdges{};
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                for (size_t a = 0; a < 3; ++a)
                {
                    if (mAdjacents[t][a] == invalid)
                    {
                        std::array<size_t, 2> directed{ tri[a], tri[(a + 1) % 3] };
                        BoundaryEdge edge(t, a, false);
                        boundaryEdges.insert(std::make_pair(directed, edge));
                    }
                }
            }

            // Extract the polygons. Each polygon is the boundary for a
            // connected component of the mesh.
            for (auto const& initialEdge : boundaryEdges)
            {
                if (!initialEdge.second.visited)
                {
                    std::vector<size_t> polygon{};
                    GetBoundaryPolygon(initialEdge.second.t, initialEdge.second.a,
                        boundaryEdges, polygon);
                    polygons.push_back(std::move(polygon));
                }
            }

            if (!duplicateEndpoints)
            {
                for (auto& polygon : polygons)
                {
                    polygon.resize(polygon.size() - 1);
                }
            }
        }

    protected:
        struct BoundaryEdge
        {
            BoundaryEdge()
                :
                t(invalid),
                a(invalid),
                visited(false)
            {
            }

            BoundaryEdge(size_t inT, size_t inA, bool inVisited)
                :
                t(inT),
                a(inA),
                visited(inVisited)
            {
            }

            size_t t;
            size_t a;
            bool visited;
        };

        using BoundaryEdgeMap = std::map<std::array<size_t, 2>, BoundaryEdge>;

        // Count the number of triangles sharing each vertex. The total number
        // of indices for triangles adjacent to vertices is 3 * numTriangles.
        // This is easy to see from the code where an increment occurs 3 times
        // per triangle.
        void GetNumTrianglesAtVertex(std::vector<size_t>& counts)
        {
            for (auto const& tri : mTriangles)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    ++counts[tri[i]];
                }
            }

            auto extremes = std::minmax_element(counts.begin(), counts.end());
            mMinTrianglesAtVertex = *extremes.first;
            mMaxTrianglesAtVertex = *extremes.second;
        }

        // Assign the storage subblocks to the vertices. The mNumVAdjacents
        // member is incremented later during a triangle traversal and is
        // used as an index into mVAdjacents during the traversal.
        void InitializeVertexStorage(std::vector<size_t> const& numTrianglesAtVertex)
        {
            auto* storage = mStorage.data();
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                mVertices[v].Initialize(numTrianglesAtVertex[v], storage);
            }
        }

        // Populate each vertex with its adjacent L-triangle, adjacent
        // vertices and outgoing edges.
        void PopulateVertices()
        {
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                size_t v0 = tri[0];
                size_t v1 = tri[1];
                size_t v2 = tri[2];

                // Update the adjacency information at v0.
                auto& vertex0 = mVertices[v0];
                vertex0.InsertVAdjacent(v1);
                vertex0.InsertVAdjacent(v2);
                vertex0.InsertEAdjacent(v1, t);

                // Update the adjacency information at v1.
                auto& vertex1 = mVertices[v1];
                vertex1.InsertVAdjacent(v2);
                vertex1.InsertVAdjacent(v0);
                vertex1.InsertEAdjacent(v2, t);

                // Update the adjacency information at v2.
                auto& vertex2 = mVertices[v2];
                vertex2.InsertVAdjacent(v0);
                vertex2.InsertVAdjacent(v1);
                vertex2.InsertEAdjacent(v0, t);
            }
        }

        // Update triangle adjacency information for edges that are shared by
        // two triangles.
        void UpdateAdjacencyForSharedEdges(size_t numThreads)
        {
            if (numThreads <= 1)
            {
                UpdateAdjacencyForSharedEdgesSingleThreaded();
            }
            else
            {
                UpdateAdjacencyForSharedEdgesMultithreaded(numThreads);
            }
        }

        void UpdateAdjacencyForSharedEdgesSingleThreaded()
        {
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                UpdateAdjacencyForTriangle(t);
            }
        }

        void UpdateAdjacencyForSharedEdgesMultithreaded(size_t numThreads)
        {
            size_t const numTriangles = mTriangles.size();
            size_t const numTrianglesPerThread = numTriangles / numThreads;
            std::vector<size_t> tmin(numThreads), tsup(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                tmin[i] = i * numTrianglesPerThread;
                tsup[i] = (i + 1) * numTrianglesPerThread;
            }
            tsup.back() = numTriangles;

            std::vector<std::thread> process(numThreads);
            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i] = std::thread([this, i, &tmin, &tsup]()
                {
                    for (size_t t = tmin[i]; t < tsup[i]; ++t)
                    {
                        UpdateAdjacencyForTriangle(t);
                    }
                });
            }

            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i].join();
            }
        }

        void UpdateAdjacencyForTriangle(size_t t)
        {
            auto const& tri = mTriangles[t];
            for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                // Get an outgoing edge <v0,v1>.
                size_t v0 = tri[i0];
                size_t v1 = tri[i1];

                // The outgoing edge from v0 is <v0,v1> and has adjacency
                // triple <v1,LT0,invalid>. If <v1,v0> is an outgoing edge
                // from v1 with adjacency triple <v0,LT1,invalid>, update
                // the v0 adjacent triple to <v1,LT0,LT1>; that is,
                // RT0 = LT1. Although it is possible at this time to update
                // the v1 adjacent triple to <v0,LT1,LT0>, where RT1 = LT0,
                // the triple will be processed when the outgoing edge is
                // visited at another time. By not updating <v0,LT1,invalid>
                // now, two writes are avoided to each of RT0 and RT1. This
                // also supports the multithreaded approach because one thread
                // never has the potential to write to the same memory
                // location that another thread writes to.
                auto* edge0 = GetOutgoingEdge(v0, v1);
                auto* edge1 = GetOutgoingEdge(v1, v0);
                if (edge0 != nullptr && edge1 != nullptr)
                {
                    (*edge0)[2] = (*edge1)[1];  // RT0 = LT1
                    mAdjacents[t][i0] = (*edge1)[1];
                }
            }
        }

        std::array<size_t, 3>* GetOutgoingEdge(size_t v0, size_t v1)
        {
            auto& vertex = mVertices[v0];
            size_t const numEAdjacents = vertex.mNumEAdjacents;
            auto* eAdjacents = vertex.mEAdjacents;
            for (size_t j = 0; j < numEAdjacents; ++j, ++eAdjacents)
            {
                if ((*eAdjacents)[0] == v1)
                {
                    return eAdjacents;
                }
            }
            return nullptr;
        }

        size_t DepthFirstSearch(size_t tInitial, std::vector<uint32_t>& visited,
            std::vector<size_t>& tStack, size_t* component) const
        {
            // The initial 'top' value denotes an empty stack. When the
            // initial value is incremented, C++ guarantees the result is 0.
            // Similarly, when 'top' is 0 and decremented, C++ guarantees the
            // result is the maximum value of size_t.
            size_t constexpr smax = std::numeric_limits<size_t>::max();
            size_t top = smax;
            size_t numInserted = 0;

            tStack[++top] = tInitial;
            while (top != smax)
            {
                size_t t = tStack[top];
                visited[t] = 1;
                size_t i{};
                for (i = 0; i < 3; ++i)
                {
                    size_t tAdjacent = mAdjacents[t][i];
                    if (tAdjacent != smax && visited[tAdjacent] == 0)
                    {
                        tStack[++top] = tAdjacent;
                        break;
                    }
                }
                if (i == 3)
                {
                    visited[t] = 2;
                    *component++ = t;
                    ++numInserted;
                    --top;
                }
            }

            return numInserted;
        }

        void GetBoundaryPolygon(size_t t, size_t a,
            BoundaryEdgeMap& boundaryEdges, std::vector<size_t>& polygon) const
        {
            std::array<size_t, 3> tri = mTriangles[t];
            size_t i0 = a;
            size_t i1 = (i0 + 1) % 3;
            std::array<size_t, 2> vEdge = { tri[i0], tri[i1] };
            polygon.push_back(vEdge[0]);
            while (!boundaryEdges[vEdge].visited)
            {
                polygon.push_back(vEdge[1]);
                boundaryEdges[vEdge].visited = true;

                // Traverse the triangle strip with vertex at vEdge[1] until
                // the last triangle is encountered. The final edge of the
                // last triangle is the next boundary edge and starts at
                // vEdge[1].
                a = mAdjacents[t][i1];
                while (a != invalid)
                {
                    // Get the next triangle in the strip.
                    t = a;
                    tri = mTriangles[t];
                    for (i1 = 0; i1 < 3; ++i1)
                    {
                        if (vEdge[1] == tri[i1])
                        {
                            // Get the next interior edge in the triangle
                            // strip, namely, <tri[i0], tri[i1]>.
                            i0 = (i1 + 1) % 3;
                            a = mAdjacents[t][i1];
                            break;
                        }
                    }
                    LogAssert(i1 < 3, "Unexpected condition.");
                }

                size_t i2 = (i1 + 1) % 3;
                vEdge[0] = vEdge[1];
                // NOTE: Microsoft Visual Studio 2022 (17.4.4) generates
                // warning C28020 for the next line of code. The code analyzer
                // believes that i2 does not satisfy 0 <= i2 <= 2. This is
                // incorrect because i2 is an unsigned integer computed
                // modulo 3.
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
#endif
                vEdge[1] = tri[i2];
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                i0 = i1;
                i1 = i2;
            }
        }

        std::vector<Vertex> mVertices;
        std::vector<size_t> mStorage;
        std::vector<std::array<size_t, 3>> mTriangles;
        std::vector<std::array<size_t, 3>> mAdjacents;
        size_t mMinTrianglesAtVertex;
        size_t mMaxTrianglesAtVertex;
    };
}
