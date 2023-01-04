// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.01.04

#pragma once

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <map>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

// StaticVETManifoldMesh2 represents a vertex-edge-triangle manifold mesh for
// which triangles are provided as a single batch and no mesh modification
// operations are going to be performed on the mesh. StaticVETManifoldMesh2
// significantly outperforms VETManifoldMesh. VETManifoldMesh is dynamic,
// allowing triangle insertions and triangle removals at any time. The
// underlying C++ container classes lead to significant memory allocation and
// deallocation costs and are also expensive for find operations. Class
// StaticVETManifoldMesh2 minimizes the memory management costs. Moreover, it
// allows for multithreading which is useful when the numbers of vertices and
// triangles are large. It is a requirement that the input triangles form a
// manifold mesh with consistently ordered triangles. In most applications,
// this requirement is already satisfied.

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

        // Compute the boundary-edge components of the mesh. These are simple
        // closed polygons. A vertex adjacency graph is computed internally. A
        // vertex with exactly 2 neighbors is the common case that is easy to
        // process. A vertex with 2n neighbors, where n > 1, is a branch point
        // of the graph. The algorithm computes n pairs of edges at a branch
        // point, each pair bounding a triangle strip whose triangles all
        // share the branch point. If you select duplicateEndpoints to be
        // false, a component has consecutive vertices
        //   (v[0], v[1], ..., v[n-1])
        // and the polygon has edges
        //   (v[0],v[1]), (v[1],v[2]), ..., (v[n-2],v[n-1]), (v[n-1],v[0])
        // If duplicateEndpoints is set to true, a component has consecutive
        // vertices
        //   (v[0], v[1], ..., v[n-1], v[0]), emphasizing that the component
        // is closed.
        void GetBoundaryPolygons(std::vector<std::vector<size_t>>& components,
            bool duplicateEndpoints) const
        {
            components.clear();

            // Build the vertex adjacency graph for the boundary edges.
            std::unordered_set<size_t> boundaryVertices{};
            std::multimap<size_t, size_t> vertexGraph{};
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    if (mAdjacents[t][i0] == invalid)
                    {
                        size_t const v0 = tri[i0];
                        size_t const v1 = tri[i1];
                        boundaryVertices.insert(v0);
                        boundaryVertices.insert(v1);
                        vertexGraph.insert(std::make_pair(v0, v1));
                        vertexGraph.insert(std::make_pair(v1, v0));
                    }
                }
            }

            // Create a set of edge pairs. For a 2-adjacency vertex v with
            // adjacent vertices v0 and v1, an edge pair is (v, v0, v1) which
            // represents undirected edges (v, v0) and (v, v1). A vertex with
            // 2n-adjacency has n edge pairs of the form (v, v0, v1). Each
            // edge pair forms the boundary of a triangle strip where each
            // triangle shares v. When traversing a boundary curve for a
            // connected component of triangles, if a 2n-adjacency vertex v
            // is encountered, let v0 be the incoming vertex. The edge pair
            // containing v and v0 is selected to generate the outgoing
            // vertex v1.
            std::multimap<size_t, std::array<size_t, 2>> edgePairs{};
            for (size_t v : boundaryVertices)
            {
                // The number of adjacent vertices is positive and even.
                size_t numAdjacents = vertexGraph.count(v);
                if (numAdjacents == 2)
                {
                    auto lbIter = vertexGraph.lower_bound(v);
                    std::array<size_t, 2> endpoints = { 0, 0 };
                    endpoints[0] = lbIter->second;
                    ++lbIter;
                    endpoints[1] = lbIter->second;
                    edgePairs.insert(std::make_pair(v, endpoints));
                }
                else
                {
                    // Create pairs of vertices that form a wedge of triangles
                    // at the vertex v, as a triangle strip of triangles all
                    // sharing v.
                    std::unordered_set<size_t> adjacents{};
                    auto lbIter = vertexGraph.lower_bound(v);
                    auto ubIter = vertexGraph.upper_bound(v);
                    for (; lbIter != ubIter; ++lbIter)
                    {
                        adjacents.insert(lbIter->second);
                    }

                    size_t const numEdgePairs = adjacents.size() / 2;
                    for (size_t j = 0; j < numEdgePairs; ++j)
                    {
                        // Get the first vertex of a pair of edges.
                        auto adjIter = adjacents.begin();
                        size_t vAdjacent = *adjIter;
                        adjacents.erase(adjIter);

                        // The wedge of triangles at v starts with a triangle
                        // that has the boundary edge.
                        size_t tCurrent{}, tOther{};
                        GetAdjacentTriangles(v, vAdjacent, tCurrent, tOther);
                        auto const& tri = mTriangles[tCurrent];

                        // Traverse the triangle strip to the other boundary
                        // edge that bounds the wedge.
                        size_t vOpposite = invalid, vStart = vAdjacent;
                        for (;;)
                        {
                            size_t i;
                            for (i = 0; i < 3; ++i)
                            {
                                vOpposite = tri[i];
                                if (vOpposite != v && vOpposite != vAdjacent)
                                {
                                    break;
                                }
                            }
                            LogAssert(i < 3, "unexpected condition");

                            size_t tNext{};
                            GetAdjacentTriangles(v, vOpposite, tOther, tNext);

                            if (tNext == invalid)
                            {
                                // Found the last triangle in the strip.
                                break;
                            }

                            // The edge is interior to the component. Traverse
                            // to the triangle adjacent to the current one.
                            tCurrent = (tNext != tCurrent ? tNext : tOther);
                            vAdjacent = vOpposite;
                        }

                        // The boundary edge of the first triangle in the
                        // wedge is (v, vAdjacent). The boundary edge of the
                        // last triangle in the wedge is (v, vOpposite).
                        std::array<size_t, 2> endpoints{ vStart, vOpposite };
                        edgePairs.insert(std::make_pair(v, endpoints));
                        adjacents.erase(vOpposite);
                    }
                }
            }

            while (edgePairs.size() > 0)
            {
                // Find the edge-pair for vStart that contains vNext and
                // remove it.
                size_t vStart = edgePairs.begin()->first;
                size_t vNext = edgePairs.begin()->second[0];
                auto lbIter = edgePairs.lower_bound(vStart);
                auto ubIter = edgePairs.upper_bound(vStart);
                bool foundStart = false;
                for (; lbIter != ubIter; ++lbIter)
                {
                    if (lbIter->second[0] == vNext || lbIter->second[1] == vNext)
                    {
                        edgePairs.erase(lbIter);
                        foundStart = true;
                        break;
                    }
                }
                LogAssert(foundStart, "unexpected condition");

                // Compute the connected component of the boundary edges that
                // contains the edge <vStart, vNext>.
                std::vector<size_t> component{};
                component.push_back(vStart);
                size_t vPrevious = vStart;
                while (vNext != vStart)
                {
                    component.push_back(vNext);

                    bool foundNext = false;
                    lbIter = edgePairs.lower_bound(vNext);
                    ubIter = edgePairs.upper_bound(vNext);
                    for (; lbIter != ubIter; ++lbIter)
                    {
                        if (vPrevious == lbIter->second[0])
                        {
                            vPrevious = vNext;
                            vNext = lbIter->second[1];
                            edgePairs.erase(lbIter);
                            foundNext = true;
                            break;
                        }
                        if (vPrevious == lbIter->second[1])
                        {
                            vPrevious = vNext;
                            vNext = lbIter->second[0];
                            edgePairs.erase(lbIter);
                            foundNext = true;
                            break;
                        }
                    }
                    LogAssert(foundNext, "unexpected condition");
                }

                if (duplicateEndpoints)
                {
                    // Explicitly duplicate the starting vertex to
                    // emphasize that the component is a closed polyline.
                    component.push_back(vStart);
                }

                components.push_back(component);
            }
        }

    protected:
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

        std::vector<Vertex> mVertices;
        std::vector<size_t> mStorage;
        std::vector<std::array<size_t, 3>> mTriangles;
        std::vector<std::array<size_t, 3>> mAdjacents;
        size_t mMinTrianglesAtVertex;
        size_t mMaxTrianglesAtVertex;
    };
}
