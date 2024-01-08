// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// MeshStaticManifold2 represents a vertex-edge-triangle manifold mesh for
// which triangles are provided as a single batch and no mesh modification
// operations are going to be performed on the mesh. MeshStaticManifold2
// significantly outperforms VETManifoldMesh. VETManifoldMesh is dynamic,
// allowing triangle insertions and removals at any time. The underlying C++
// container classes lead to significant memory allocation and deallocation
// costs and are also expensive for find operations. MeshStaticManifold2
// minimizes the memory management costs. Moreover, it allows for
// multithreading which is useful when the numbers of vertices and triangles
// are large. It is a requirement that the input triangles form a manifold
// mesh with consistently ordered triangles. In most applications, this
// requirement is already satisfied.

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
    class MeshStaticManifold2
    {
    public:
        // Use the maximum size_t to denote an invalid index, effectively
        // representing -1.
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        // The vertices are stored as std::vector<Vertex> vertex(numVertices).
        // If triangle[t0] = <v0,v1,v2>, then vertex[v0] contains a 4-tuple
        // {v1,v2,t0,a0}. The undirected edge (v1,v2) is opposite v0. If there
        // is no adjacent triangle sharing (v1,v2), then a0 is invalid. If
        // there is an adjacent triangle, then a0 is the index for that
        // triangle. Let triangle[a0] = <v2,v1,v3>; then vertex[v3] contains
        // a 4-tuple {v2,v1,a0,t0}.
        // 
        // The member pointers of Vertex are addresses into a contiguous block
        // of memory in order to minimize the costs of memory management. The
        // block of memory has 12 * numTriangles elements of type size_t.
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

            inline std::array<size_t, 4> const* GetAdjacents() const
            {
                return mAdjacents;
            }

        private:
            // Only MeshStaticManifold2 may write the members of this class.
            friend class MeshStaticManifold2;

            void Initialize(size_t numAdjacents, size_t*& storage)
            {
                mNumAdjacents = 0;
                mAdjacents = reinterpret_cast<std::array<size_t, 4>*>(storage);
                storage += 4 * numAdjacents;
            }

            void Insert(size_t v1, size_t v2, size_t t, size_t location)
            {
                mAdjacents[mNumAdjacents++] = { v1, v2, t, location };
            }

            // The number of triangles sharing v0. The value starts at zero
            // and is incremented during the MeshStaticManifold2::Populate
            // execution.
            size_t mNumAdjacents;

            // If triangle t0 is <v0,v1,v2> in counterclockwise order, then
            // the corresponding adjacents element is {v1,v2,t0,a0}, where
            // a0 is invalid when (v1,v2) is contained by a single triangle
            // or a0 is the index for the adjacent triangle when (v1,v2) is
            // contained by two triangles.
            std::array<size_t, 4>* mAdjacents;
        };

        // Preconditions.
        //   1. The triangles input must have size 1 or larger.
        //   2. The number of vertices must be 3 or larger.
        //   3. The triangles must form a manifold mesh.
        //   4. Each triangle must be nondegenerate; no repeated vertices.
        //   5. The triangles must all be ordered counterclockwise.
        // Set numThreads to 2 or larger to activate multithreading in the
        // mesh construction. If numThreads is 0 or 1, the construction
        // occurs in the main thread.
        MeshStaticManifold2(
            size_t numVertices,
            std::vector<std::array<size_t, 3>> const& triangles,
            size_t numThreads)
            :
            mVertices(numVertices),
            mStorage(12 * triangles.size(), invalid),
            mTriangles(triangles),
            mAdjacents(triangles.size(), { invalid, invalid, invalid }),
            mMinTrianglesAtVertex(0),
            mMaxTrianglesAtVertex(0)
        {
            LogAssert(numVertices >= 3 && triangles.size() > 0, "Invalid input.");

            std::vector<size_t> numTrianglesAtVertex(numVertices, 0);
            GetNumTrianglesAtVertex(numTrianglesAtVertex);
            InitializeStorage(numTrianglesAtVertex);
            Populate();
            UpdateAdjacencyForSharedEdges(numThreads);
        }

        // Member access.
        inline std::vector<Vertex> const& GetVertices() const
        {
            return mVertices;
        }

        // Each 3-tuple contains indices into the vertices.
        inline std::vector<std::array<size_t, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        // Each 3-tuple contains indices into the triangles.
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
                return GetDirectedEdge(v0, v1) != nullptr
                    || GetDirectedEdge(v1, v0) != nullptr;
            }
            return false;
        }

        // Get the adjacent triangles for the undirected edge (v0,v1). The
        // returned adjacent triangle indices adj0 and adj1 are the following:
        //
        //   1. <v0,v1> and <v1,v0> are both directed edges, so the edge is
        //      shared by two triangles and both adj0 and adj1 are valid (not
        //      equal to invalid). The index adj0 is the L-triangle for
        //      <v0,v1> and the index adj1 is the R-triangle for <v0,v1>.
        //      Equivalently, adj0 is the R-triangle for <v1,v0> and adj1 is
        //      the L-triangle for <v1,v0>.
        // 
        //   2. <v0,v1> is directed but <v1,v0> does not exist. The index
        //      adj0 is the L-triangle for <v0,v1> and the index adj1 is
        //      invalid (no R-triangle).
        // 
        //   3. <v1,v0> is directed but <v0,v1> does not exist. The index
        //      adj0 is invalid (no L-triangle) for <v1,v0> and the index
        //      adj1 is the R-triangle for <v1,v0>.
        // 
        //   4. Neither <v0,v1> nor <v1,v0> exist; that is, the edge does
        //      not occur for any triangle.
        //
        // It is possible to distinguish among the 4 cases by examining the
        // returned indices:
        //   (1) returns (valid, valid) and Boolean 'true'
        //   (2) returns (valid, invalid) and Boolean 'true'
        //   (3) returns (invalid, valid) and Boolean 'true'
        //   (4) returns (invalid, invalid) and Boolean 'false'
        bool GetAdjacentTriangles(size_t v0, size_t v1, size_t& adj0, size_t& adj1) const
        {
            if (v0 < mVertices.size() && v1 < mVertices.size() && v0 != v1)
            {
                auto const* adjacents0 = GetDirectedEdge(v0, v1);
                if (adjacents0 != nullptr)
                {
                    adj0 = (*adjacents0)[2];
                    adj1 = (*adjacents0)[3];
                    return true;
                }

                auto const* adjacents1 = GetDirectedEdge(v1, v0);
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
                        std::array<size_t, 2> directed{ tri[(a + 1) % 3], tri[(a + 2) % 3] };
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

    private:
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
        // per triangle. Each adjacent element has 4 components of type
        // size_t, so the adjacent storage requires 12 * numTriangles values
        // of type size_t.
        void GetNumTrianglesAtVertex(std::vector<size_t>& counts)
        {
            for (auto const& tri : mTriangles)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    ++counts[tri[i]];
                }
            }

            // The minimum and maximum triangle counts are for statistical
            // information.
            auto extremes = std::minmax_element(counts.begin(), counts.end());
            mMinTrianglesAtVertex = *extremes.first;
            mMaxTrianglesAtVertex = *extremes.second;
        }

        // Assign the storage subblocks to the vertices.
        void InitializeStorage(std::vector<size_t> const& numTrianglesAtVertex)
        {
            auto* storage = mStorage.data();
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                mVertices[v].Initialize(numTrianglesAtVertex[v], storage);
            }
        }

        // Populate the adjacency information for the vertices. The number of
        // vertex[v] adjacents is 3 * numTrianglesAtVertex[v]. This requires
        // 12 * numTrianglesAtVertex[v] elements of type size_t. For the
        // entire mesh, we need 12 * numTriangles elements of type size_t.
        void Populate()
        {
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                size_t v0 = tri[0];
                size_t v1 = tri[1];
                size_t v2 = tri[2];

                // The last arguments (i = 0, 1 or 2) are used to set the
                // correct mAdjacents[][i] indices. These arguments are
                // replaced later by the actual indices for adjacent triangles
                // sharing the edge.
                mVertices[v0].Insert(v1, v2, t, 0);
                mVertices[v1].Insert(v2, v0, t, 1);
                mVertices[v2].Insert(v0, v1, t, 2);
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
            for (size_t v = 0; v < mVertices.size(); ++v)
            {
                UpdateAdjacencyForEdge(v);
            }
        }

        void UpdateAdjacencyForSharedEdgesMultithreaded(size_t numThreads)
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
                        UpdateAdjacencyForEdge(v);
                    }
                });
            }

            for (size_t i = 0; i < numThreads; ++i)
            {
                process[i].join();
            }
        }

        void UpdateAdjacencyForEdge(size_t v0)
        {
            auto& vertex0 = mVertices[v0];
            auto* adjacents0 = vertex0.mAdjacents;
            for (size_t j = 0; j < vertex0.mNumAdjacents; ++j, ++adjacents0)
            {
                size_t v1 = (*adjacents0)[0];
                size_t v2 = (*adjacents0)[1];

                // The edge opposite vertex v0 is (v1,v2). We know that
                // vertex[v0] contains a 4-tuple {v1,v2,tri0,loc0}. Determine
                // whether vertex[v2] contains a 4-tuple {v1,v3,adj1,loc1}.
                auto* adjacents1 = GetDirectedEdge(v2, v1);
                if (adjacents1)
                {
                    // The edge <v1,v2> has a triangle adjacent to triangle
                    // tri0. Update the vertex adjacency information for
                    // triangle tri0 at that edge. Triangle a1 adjacency is
                    // not updated. It will be updated when <v2,v1> is visited
                    // at another time. This avoids two writes of the adjacent
                    // triangle indices. It also supports the multithreaded
                    // approach because one thread never has the potential to
                    // write to the same memory location that another thread
                    // writes to.
                    size_t tri0 = (*adjacents0)[2];
                    size_t loc0 = (*adjacents0)[3];
                    size_t adj1 = (*adjacents1)[2];
                    (*adjacents0)[3] = adj1;
                    mAdjacents[tri0][loc0] = adj1;
                }
                else
                {
                    // Replace the mAdjacents[] location value (0, 1 or 2) by
                    // an invalid index because edge <v1,v0> does not exist,
                    // in which case there is no adjacent triangle to edge
                    // <v0,v1>.
                    (*adjacents0)[3] = invalid;
                }
            }
        }

        std::array<size_t, 4>* GetDirectedEdge(size_t v0, size_t v1) const
        {
            auto& vertex0 = mVertices[v0];
            auto* adjacents0 = vertex0.mAdjacents;
            for (size_t j = 0; j < vertex0.mNumAdjacents; ++j, ++adjacents0)
            {
                if ((*adjacents0)[0] == v1)
                {
                    return adjacents0;
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
            size_t top = invalid;
            size_t numInserted = 0;

            tStack[++top] = tInitial;
            while (top != invalid)
            {
                size_t t = tStack[top];
                visited[t] = 1;
                size_t i{};
                for (i = 0; i < 3; ++i)
                {
                    size_t tAdjacent = mAdjacents[t][i];
                    if (tAdjacent != invalid && visited[tAdjacent] == 0)
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
            size_t i0 = (a + 1) % 3;
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
                a = mAdjacents[t][i0];
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
                            i0 = (i1 + 2) % 3;
                            a = mAdjacents[t][i0];
                            break;
                        }
                    }
                    LogAssert(i1 < 3, "Unexpected condition.");
                }

                size_t i2 = (i1 + 1) % 3;
                vEdge[0] = vEdge[1];
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
#endif
                // NOTE: Microsoft Visual Studio 2022 (17.4.4) generates
                // warning C28020 for the next line of code. The code analyzer
                // believes that i2 does not satisfy 0 <= i2 <= 2. This is
                // incorrect because i2 is an unsigned integer computed
                // modulo 3.
                vEdge[1] = tri[i2];
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                i0 = i1;
                i1 = (i0 + 1) % 3;
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
