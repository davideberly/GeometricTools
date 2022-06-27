// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.5.2022.06.26

#pragma once

#include <Mathematics/Logger.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <unordered_set>
#include <vector>

// VETManifoldMeshNR represents a vertex-edge-triangle manifold mesh for which
// triangles are provided as a single batch and NO REMOVE operations are going
// to be performed on the mesh. VETManifoldMeshNR significantly outperforms
// VETManifoldMesh. VETManifoldMesh is general purpose, allowing triangle
// insertions and triangle removals. The underlying C++ container classes lead
// to significant memory allocation/deallocation costs and are also expensive
// for find operations. It turns out that the design of VETManifoldMeshNR
// automatically gives you vertex adjacency information, so there is no
// ETManifoldMeshNR implementation. It is a requirement that the input
// triangles form a manifold mesh with consistently ordered triangles. In most
// applications, this requirement is already satisfied.
// 
// The VETManifoldMeshNR class is designed to minimize the allocations. For
// example, using this class for a collection of 1131652 positions and 2242293
// triangles, the CPU times on an Intel (R) Core (TM) i9-10900 are as follows.
//
// ETManifoldMesh (insert all triangles, no removals)
//   graph creation:                2234 milliseconds
//   connected component labeling:  2169 milliseconds
//   get boundary polygons:          232 milliseconds
//
// VETManifoldMeshNR
//   graph creation:                 118 milliseconds
//   connected component labeling:    40 milliseconds
//   get boundary polygons:           25 milliseconds

namespace gte
{
    class VETManifoldMeshNR
    {
    public:
        // Use the maximum size_t to denote an invalid number.
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        class Vertex
        {
        public:
            Vertex()
                :
                mNumOutAdjacents(0),
                mOutAdjacents(nullptr),
                mNumInAdjacents(0),
                mInAdjacents(nullptr)
            {
            }

            // Read-only access to all the adjacent vertex-triangle pairs.
            inline size_t GetNumAdjacents() const
            {
                return mNumOutAdjacents;  // = mNumInAdjacents
            }

            inline std::array<size_t, 2> const* GetAdjacents() const
            {
                return mOutAdjacents;
            }


            // Read-only access to outgoing-adjacent vertex-triangle pairs.
            inline size_t GetNumOutAdjacents() const
            {
                return mNumOutAdjacents;
            }

            inline std::array<size_t, 2> const* GetOutAdjacents() const
            {
                return mOutAdjacents;
            }

            inline size_t GetOutV(size_t i) const
            {
                LogAssert(i < mNumOutAdjacents, "Index is out of range.");
                return mOutAdjacents[i][0];
            }

            inline size_t GetOutT(size_t i) const
            {
                LogAssert(i < mNumOutAdjacents, "Index is out of range.");
                return mOutAdjacents[i][1];
            }

            inline std::array<size_t, 2> GetOutVT(size_t i) const
            {
                LogAssert(i < mNumOutAdjacents, "Index is out of range.");
                return mOutAdjacents[i];
            }

            // Read-only access to incoming-adjacent vertex-triangle pairs.
            inline size_t GetNumInAdjacents() const
            {
                return mNumInAdjacents;
            }

            inline std::array<size_t, 2> const* GetInAdjacents() const
            {
                return mInAdjacents;
            }

            inline size_t GetInV(size_t i) const
            {
                LogAssert(i < mNumInAdjacents, "Index is out of range.");
                return mInAdjacents[i][0];
            }

            inline size_t GetInT(size_t i) const
            {
                LogAssert(i < mNumInAdjacents, "Index is out of range.");
                return mInAdjacents[i][1];
            }

            inline std::array<size_t, 2> GetInVT(size_t i) const
            {
                LogAssert(i < mNumInAdjacents, "Index is out of range.");
                return mInAdjacents[i];
            }

        private:
            // Only VETManifoldMeshNR should write the members of this class.
            friend class VETManifoldMeshNR;

            // The mOutAdjacents pointer is to a contiguous block of memory
            // that stores 2*mNumAdjacents <V,T> pairs. The outgoing pairs are
            // stored first followed by the incoming pairs are stored after.
            // Note that mInAdjacents = mOutAdjacents + mNumOutAdjacents.
            size_t mNumOutAdjacents;
            std::array<size_t, 2>* mOutAdjacents;
            size_t mNumInAdjacents;
            std::array<size_t, 2>* mInAdjacents;
        };

        // Preconditions.
        //   1. The number of vertices must be 3 or larger (at least one
        //      triangle must exist).
        //   2. The triangles must form a manifold mesh.
        //   3. The triangle must be nondegenerate (no repeated vertices).
        //   4. The triangles must all be ordered counterclockwise or all
        //      ordered clockwise (no mixed chirality).
        //   5. The vertex indices must be nonnegative.
        VETManifoldMeshNR(size_t numVertices,
            std::vector<std::array<size_t, 3>> const& triangles)
            :
            mVertices(numVertices),
            mVertexStorage{},
            mTriangles(triangles),
            mAdjacents(triangles.size())
        {
            LogAssert(numVertices >= 3, "Invalid number of vertices.");

            // Count the number of outgoing edges from each vertex. The number
            // of incoming edges is the same.
            std::vector<size_t> numEdgesAtVertex(numVertices, 0);
            for (auto const& tri : mTriangles)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    ++numEdgesAtVertex[tri[i]];
                }
            }

            // Each triangle has 3 incoming edges per vertex and 3 outgoing
            // edges per vertex. Allocate storage for pairs <V,T> where V
            // is an index into the vertices and T is an index into the
            // triangles. When triangle T is processed, V is a vertex of
            // that triangle.
            mVertexStorage.resize(6 * mTriangles.size());

            // Create the directed edges. The numAdjacents member is set to 0
            // so that it can be used as an index to the next available slot
            // to write an adjacent vertex-triangle pair.
            std::array<size_t, 2>* storage = mVertexStorage.data();
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                size_t numAdjacents = numEdgesAtVertex[v];

                mVertices[v].mNumOutAdjacents = 0;
                mVertices[v].mOutAdjacents = storage;
                storage += numAdjacents;

                mVertices[v].mNumInAdjacents = 0;
                mVertices[v].mInAdjacents = storage;
                storage += numAdjacents;
            }

            // Populate the vertices with the adjacent vertex-triangle pairs.
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    size_t const v0 = tri[i0];
                    size_t const v1 = tri[i1];

                    // Write an outgoing vertex-triangle pair to storage.
                    auto& outVertex = mVertices[v0];
                    outVertex.mOutAdjacents[outVertex.mNumOutAdjacents++] = { v1, t };

                    // Write an incoming vertex-triangle pair to storage.
                    auto& inVertex = mVertices[v1];
                    inVertex.mInAdjacents[inVertex.mNumInAdjacents++] = { v0, t };
                }
            }

            // Process the edge-triangle graph to determine the adjacent
            // triangles for each mesh triangle.
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
                {
                    size_t const v0 = tri[i0];
                    size_t const v1 = tri[i1];

                    // If there is an adjacent triangle for edge <v0,v1>,
                    // that triangle has an edge <v1,v0>.
                    auto const& outVertex = mVertices[v1];
                    mAdjacents[t][i0] = invalid;
                    for (size_t j = 0; j < outVertex.mNumOutAdjacents; ++j)
                    {
                        if (outVertex.mOutAdjacents[j][0] == v0)
                        {
                            mAdjacents[t][i0] = outVertex.mOutAdjacents[j][1];
                        }
                    }
                }
            }
        }

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

        void GetAdjacentTriangles(size_t v0, size_t v1, size_t& adj0, size_t& adj1) const
        {
            LogAssert(v0 < mVertices.size() && v1 < mVertices.size(), "Invalid index.");

            adj0 = invalid;
            adj1 = invalid;

            auto const& vertex0 = mVertices[v0];
            for (size_t i = 0; i < vertex0.GetNumOutAdjacents(); ++i)
            {
                auto vtPair = vertex0.GetOutVT(i);
                if (v1 == vtPair[0])
                {
                    adj0 = vtPair[1];
                }
            }
            LogAssert(adj0 != invalid, "Unexpected condition.");

            auto const& vertex1 = mVertices[v1];
            for (size_t i = 0; i < vertex1.GetNumOutAdjacents(); ++i)
            {
                auto vtPair = vertex1.GetOutVT(i);
                if (v0 == vtPair[0])
                {
                    adj1 = vtPair[1];
                }
            }
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

        // Vertex and edge information. The vertices store adjacency data
        // including adjacent-vertex and adjacent-triangle information.
        // The edge information is indirectly stored in the vertices.
        std::vector<Vertex> mVertices;
        std::vector<std::array<size_t, 2>> mVertexStorage;

        // Triangle information. The mTriangles[] indices are lookups into the
        // vertices. The mAdjacents[] indices are lookups into mTriangles[].
        std::vector<std::array<size_t, 3>> mTriangles;
        std::vector<std::array<size_t, 3>> mAdjacents;
    };
}
