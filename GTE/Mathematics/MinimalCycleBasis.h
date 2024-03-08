// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.01.03

#pragma once

// Extract the minimal cycle basis for a planar graph. The input vertices and
// edges must form a graph for which edges intersect only at vertices; that
// is, no two edges must intersect at an interior point of one of the edges.
// The algorithm is described in 
//   https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf
// The graph might have isolated vertices (no adjacent vertices via edges).
// These are extracted by the implementation. The graph might have filaments,
// which are subgraphcs of polylines that are not shared by a cycle. These are
// also extracted by the implementation.

#include <Mathematics/ArbitraryPrecision.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <type_traits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T, typename IndexType>
    class MinimalCycleBasis
    {
    public:
        struct Tree
        {
            Tree()
                :
                cycle{},
                children{}
            {
            }

            std::vector<IndexType> cycle;
            std::vector<std::shared_ptr<Tree>> children;
        };

        using Position = std::array<T, 2>;
        using Edge = std::array<IndexType, 2>;
        using Forest = std::vector<std::shared_ptr<Tree>>;
        using Filament = std::vector<IndexType>;

        // The input positions and edges must form a planar graph for which
        // edges intersect only at vertices; that is, no two edges must
        // intersect at an interior point of one of the edges.
        MinimalCycleBasis()
            :
            mVertices{},
            mIsolatedVertices{},
            mFilaments{},
            mForest{}
        {
            static_assert(std::is_floating_point<T>::value,
                "Type T must be 'float' or 'double'.");

            static_assert(std::is_integral<IndexType>::value && sizeof(IndexType) >= 2,
                "IndexType must be a signed or unsigned integer type of size at least 2 bytes.");
        }

        // No copy semantics.
        MinimalCycleBasis(MinimalCycleBasis const&) = delete;
        MinimalCycleBasis& operator=(MinimalCycleBasis const&) = delete;

        // No move semantics.
        MinimalCycleBasis(MinimalCycleBasis&&) = delete;
        MinimalCycleBasis& operator=(MinimalCycleBasis&&) = delete;

        // Extract the cycles, filaments, and isolated vertices.
        void Extract(
            std::vector<std::array<T, 2>> const& positions,
            std::vector<std::array<IndexType, 2>> const& edges,
            bool verifyInputs)
        {
            mVertices.clear();
            mIsolatedVertices.clear();
            mFilaments.clear();
            mForest.clear();

            if (positions.size() == 0 || edges.size() == 0)
            {
                // The graph is empty, so there are no filaments or cycles.
                return;
            }

            if (verifyInputs)
            {
                VerifyInputs(positions, edges);
            }

            CreateGraph(positions, edges);

            // Extract the isolated vertices of the graph.
            ExtractIsolatedVertices();

            // Extract the tree of cycles of the graph. The filaments are
            // extracted during graph traversal.
            ExtractForest();
        }

        // Access to the output of the Extract function.
        inline std::vector<IndexType> const& GetIsolatedVertices() const
        {
            return mIsolatedVertices;
        }

        inline std::vector<Filament> const& GetFilaments() const
        {
            return mFilaments;
        }

        inline Forest const& GetForest() const
        {
            return mForest;
        }

    private:
        static void VerifyInputs(
            std::vector<Position> const& positions,
            std::vector<Edge> const& edges)
        {
            std::set<Position> uniquePositions{};
            for (auto const& position : positions)
            {
                uniquePositions.insert(position);
            }
            LogAssert(
                uniquePositions.size() == positions.size(),
                "Input positions must be unique.");

            IndexType numPositions = static_cast<IndexType>(positions.size());
            for (size_t i = 0; i < edges.size(); ++i)
            {
                auto const& edge = edges[i];
                LogAssert(
                    0 <= edge[0] && edge[0] < numPositions,
                    "Input index edge[" + std::to_string(i) + "][0] is out of range.");
                LogAssert(
                    0 <= edge[1] && edge[1] < numPositions,
                    "Input index edge[" + std::to_string(i) + "][1] is out of range.");

                LogAssert(
                    edge[0] != edge[1],
                    "Input edge[" + std::to_string(i) + "] is degenerate.");
            }
        }

    private:
        // Support for exact rational arithmetic when determining convexity at a
        //  vertex. The number of words is for worst-case storage requirements.
        static size_t constexpr numWords = std::is_same<T, float>::value ? 18 : 132;
        using Rational = BSNumber<UIntegerFP32<numWords>>;
        using RPosition = std::array<Rational, 2>;

        static RPosition Sub(RPosition const& rInput0, RPosition const& rInput1)
        {
            return RPosition{ rInput0[0] - rInput1[0], rInput0[1] - rInput1[1] };
        }

        static int32_t GetSignDet(RPosition const& rInput0, RPosition const& rInput1)
        {
            Rational rDet = rInput0[0] * rInput1[1] - rInput0[1] * rInput1[0];
            return rDet.GetSign();
        }

        struct Vertex
        {
            Vertex(IndexType inIndex = 0, Position const* inPosition = nullptr)
                :
                index(inIndex),
                position(inPosition),
                adjacents{},
                visited(0),
                rPosition{},
                rPositionComputed(0)
            {
            }

            // The index into the 'positions' provided to the call to Extract.
            IndexType index;

            // The position of the vertex, stored as a floating-point tuple.
            Position const* position;

            // The vertices adjacent to this vertex in the vertex-edge graph.
            std::set<Vertex*> adjacents;

            // Support for depth-first traversal of a graph.
            uint32_t visited;

            // The position of the vertex, stored as a rational tuple. Use
            // GetRPosition for memoized conversion of a floating-point tuple
            // to a rational tuple.
            mutable RPosition rPosition;
            mutable uint32_t rPositionComputed;

            RPosition const& GetRPosition() const
            {
                if (rPositionComputed == 0)
                {
                    rPosition[0] = (*position)[0];
                    rPosition[1] = (*position)[1];
                    rPositionComputed = 1;
                }
                return rPosition;
            }
        };

        // Create the vertex-edge graph. The edges are undirected.
        void CreateGraph(
            std::vector<Position> const& positions,
            std::vector<Edge> const& edges)
        {
            mVertices.resize(positions.size());
            IndexType index = 0;
            for (auto const& position : positions)
            {
                mVertices[index] = std::make_shared<Vertex>(index, &position);
                ++index;
            }

            for (auto const& edge : edges)
            {
                Insert(mVertices[edge[0]].get(), mVertices[edge[1]].get());
            }
        }

        // Insert an edge into the graph.
        void Insert(Vertex* vertex0, Vertex* vertex1)
        {
            vertex0->adjacents.insert(vertex1);
            vertex1->adjacents.insert(vertex0);
        }

        // Remove an edge from the graph.
        void Remove(Vertex* vertex0, Vertex* vertex1)
        {
            vertex0->adjacents.erase(vertex1);
            vertex1->adjacents.erase(vertex0);
        }

        // Extract the isolated vertices for the vertex-edge graph.
        void ExtractIsolatedVertices()
        {
            for (auto& vertex : mVertices)
            {
                if (vertex->adjacents.size() == 0)
                {
                    mIsolatedVertices.push_back(vertex->index);
                }
            }
        }

        // Extract the top-level filaments for the vertex-edge graph.
        void ExtractFilaments(std::vector<Vertex*>& component)
        {
            // Locate all filament endpoints, which are vertices with each
            // having exactly one adjacent vertex.
            std::vector<Vertex*> endpoints{};
            for (auto vertex : component)
            {
                if (vertex->adjacents.size() == 1)
                {
                    endpoints.push_back(vertex);
                }
            }
            if (endpoints.size() == 0)
            {
                // The vertex-edge graph has no filaments.
                return;
            }

            // Remove the filaments from the vertex-edge graph. The greedy
            // removal of vertices allows for removing filaments from a
            // subgraph of filaments that has branch points.
            for (auto endpoint : endpoints)
            {
                Vertex* vertex0 = endpoint;
                if (vertex0->adjacents.size() == 0)
                {
                    // The endpoint was visited during another filament
                    // traversal.
                    continue;
                }

                // Traverse the filament and remove the vertices.
                Filament filament{};
                filament.push_back(vertex0->index);
                while (vertex0->adjacents.size() == 1)
                {
                    Vertex* vertex1 = *vertex0->adjacents.begin();
                    filament.push_back(vertex1->index);
                    Remove(vertex0, vertex1);
                    vertex0 = vertex1;
                }

                // The traversal has terminated because the final vertex is
                // either an endpoint (1 adjacent) or a branch point (at least
                // 3 adjacents). When it is an endpoint, the removal in the
                // while-loop reduced the adjacent count to 0. When it is a
                // branch point, the removal in the while-loop reduced the
                // adjacent count to at least 2.
                mFilaments.emplace_back(filament);
            }

            // At this time the component is either empty because it was an
            // open polyline or it has no filaments and at least one cycle.
            // Identify the remaining vertices and copy to the component,
            // which then has fewer vertices than before the call to
            // ExtractFilaments.
            std::vector<Vertex*> remaining{};
            remaining.reserve(component.size());
            for (auto vertex : component)
            {
                if (vertex->adjacents.size() > 0)
                {
                    remaining.push_back(vertex);
                }
            }
            component = std::move(remaining);
        }

        // Extract the minimal cycle basis for the vertex-edge graph,
        // stored as a forest of trees.
        void ExtractForest()
        {
            std::vector<std::vector<Vertex*>> components{};
            ExtractConnectedComponents(components);
            for (auto& component : components)
            {
                auto tree = ExtractBasis(component);
                if (tree->children.size() > 0 || tree->cycle.size() > 0)
                {
                    mForest.push_back(tree);
                }
            }
        }

        // Extract the connected components of the graph using a
        // depth-first search. The 'visited' flags are 0 (unvisited),
        // 1 (discovered), or 2 (finished).
        void ExtractConnectedComponents(std::vector<std::vector<Vertex*>>& components)
        {
            for (auto const& vertex : mVertices)
            {
                if (vertex->adjacents.size() >= 2 && vertex->visited == 0)
                {
                    std::vector<Vertex*> component{};
                    DepthFirstSearch(vertex.get(), component);
                    components.emplace_back(component);
                }
            }

            // The depth-first search is used later for collecting
            // vertices for subgraphs that are detached from the main
            // graph, so the 'visited' flags must be reset to zero after
            // component finding.
            for (auto const& vertex : mVertices)
            {
                vertex->visited = 0;
            }
        }

        void DepthFirstSearch(Vertex* vInitial, std::vector<Vertex*>& component)
        {
            std::stack<Vertex*> vStack{};
            vStack.push(vInitial);
            while (vStack.size() > 0)
            {
                Vertex* vertex = vStack.top();
                vertex->visited = 1;
                size_t i = 0;
                for (auto adjacent : vertex->adjacents)
                {
                    if (adjacent->visited == 0)
                    {
                        vStack.push(adjacent);
                        break;
                    }
                    ++i;
                }

                if (i == vertex->adjacents.size())
                {
                    vertex->visited = 2;
                    component.push_back(vertex);
                    vStack.pop();
                }
            }
        }

        // Extract the minimal cycle basis for a connected component.
        std::shared_ptr<Tree> ExtractBasis(std::vector<Vertex*>& component)
        {
            // The top-level tree will not have its cycle member set. The
            // children are the cycle trees extracted from the component.
            auto tree = std::make_shared<Tree>();

            while (component.size() > 0)
            {
                ExtractFilaments(component);
                if (component.size() > 0)
                {
                    tree->children.push_back(ExtractCycleFromComponent(component));
                }
            }

            if (tree->cycle.size() == 0 && tree->children.size() == 1)
            {
                // Replace the parent by the child to avoid having two empty
                // cycles in parent/child.
                auto child = tree->children.back();
                tree->cycle = std::move(child->cycle);
                tree->children = std::move(child->children);
            }

            return tree;
        }

        std::shared_ptr<Tree> ExtractCycleFromComponent(std::vector<Vertex*>& component)
        {
            std::shared_ptr<Tree> tree{};

            // Search for the left-most vertex of the component. If two or
            // more vertices attain minimum x-value, select the one that has
            // minimum y-value.
            Vertex* minVertex = component[0];
            for (auto vertex : component)
            {
                if (*vertex->position < *minVertex->position)
                {
                    minVertex = vertex;
                }
            }

            // Traverse the closed walk, duplicating the starting vertex as
            // the last vertex.
            std::vector<Vertex*> closedWalk{};
            Vertex* vCurr = minVertex;
            Vertex* vStart = vCurr;
            closedWalk.push_back(vStart);
            Vertex* vAdj = GetClockwiseMost(nullptr, vStart);
            while (vAdj != vStart)
            {
                closedWalk.push_back(vAdj);
                Vertex* vNext = GetCounterclockwiseMost(vCurr, vAdj);
                vCurr = vAdj;
                vAdj = vNext;
            }
            closedWalk.push_back(vStart);

            // Recursively process the closed walk to extract cycles.
            tree = ExtractCycleFromClosedWalk(closedWalk);

            // The isolated vertices generated by cycle removal are also
            // removed from the component.
            std::vector<Vertex*> remaining{};
            remaining.reserve(component.size());
            for (auto vertex : component)
            {
                if (vertex->adjacents.size() > 0)
                {
                    remaining.push_back(vertex);
                }
            }
            component = std::move(remaining);

            return tree;
        }

        std::shared_ptr<Tree> ExtractCycleFromClosedWalk(std::vector<Vertex*>& closedWalk)
        {
            auto tree = std::make_shared<Tree>();

            std::map<Vertex*, size_t> duplicates{};
            std::set<size_t> detachments{};
            size_t numClosedWalk = closedWalk.size();
            for (size_t i = 1; i < numClosedWalk - 1; ++i)
            {
                auto diter = duplicates.find(closedWalk[i]);
                if (diter == duplicates.end())
                {
                    // We have not yet visited this vertex.
                    duplicates.insert(std::make_pair(closedWalk[i], i));
                    continue;
                }

                // The vertex has been visited previously. Collapse the closed
                // walk by removing the subwalk sharing this vertex. Note that
                // the vertex is pointed to by closedWalk[diter->second] and
                // closedWalk[i].
                size_t iMin = diter->second, iMax = i;
                detachments.insert(iMin);
                for (size_t j = iMin + 1; j < iMax; ++j)
                {
                    Vertex* vertex = closedWalk[j];
                    duplicates.erase(vertex);
                    detachments.erase(j);
                }
                closedWalk.erase(closedWalk.begin() + iMin + 1, closedWalk.begin() + iMax + 1);
                numClosedWalk = closedWalk.size();
                i = iMin;
            }

            if (numClosedWalk > 3)
            {
                // It is not known whether closedWalk[0] is a detachment
                // point. To determine this, test for any edges strictly
                // contained in the wedge formed by the edges
                // <closedWalk[0],closedWalk[N-1]> and
                // <closedWalk[0],closedWalk[1]>. However, this test must
                // be executed even for the known detachment points. The
                // ensuing logic is designed to handle this and reduce the
                // amount of code, so insert closedWalk[0] into the
                // detachment set and ignore it later if it actually is
                // not.
                detachments.insert(0);

                // Detach subgraphs from the vertices of the cycle.
                for (auto i : detachments)
                {
                    Vertex* orgVertex = closedWalk[i];
                    Vertex* maxVertex = closedWalk[i + 1];
                    Vertex* minVertex = (i > 0 ? closedWalk[i - 1] : closedWalk[numClosedWalk - 2]);

                    RPosition const& rOrgPos = orgVertex->GetRPosition();
                    RPosition rDMax = Sub(maxVertex->GetRPosition(), rOrgPos);
                    RPosition rDMin = Sub(minVertex->GetRPosition(), rOrgPos);

                    bool isConvex = (GetSignDet(rDMax, rDMin) >= 0);
                    std::set<Vertex*> inWedge{};
                    for (auto vertex : orgVertex->adjacents)
                    {
                        if (vertex->index == minVertex->index ||
                            vertex->index == maxVertex->index)
                        {
                            continue;
                        }

                        RPosition rDVer = Sub(vertex->GetRPosition(), rOrgPos);
                        int32_t signDet0 = GetSignDet(rDVer, rDMin);
                        int32_t signDet1 = GetSignDet(rDVer, rDMax);
                        bool containsVertex{};
                        if (isConvex)
                        {
                            containsVertex = (signDet0 > 0 && signDet1 < 0);
                        }
                        else
                        {
                            containsVertex = (signDet0 > 0 || signDet1 < 0);
                        }

                        if (containsVertex)
                        {
                            inWedge.insert(vertex);
                        }
                    }

                    if (inWedge.size() > 0)
                    {
                        // The clone will manage the adjacents for
                        // orgVertex that lie inside the wedge defined by
                        // the first and last edges of the subgraph rooted
                        // at orgVertex. The sorting is in the clockwise
                        // direction.
                        auto clone = std::make_shared<Vertex>(orgVertex->index, orgVertex->position);
                        mVertices.push_back(clone);

                        // Detach the edges inside the wedge.
                        for (auto vertex : inWedge)
                        {
                            Remove(vertex, orgVertex);
                            Insert(vertex, clone.get());
                        }

                        // Get the subgraph (it is a single connected
                        // component).
                        std::vector<Vertex*> component{};
                        DepthFirstSearch(clone.get(), component);

                        // Extract the cycles of the subgraph.
                        tree->children.push_back(ExtractBasis(component));
                    }
                    // else the candidate was closedWalk[0] and it has no
                    // subgraph to detach.
                }

                tree->cycle = std::move(ExtractCycle(closedWalk));
            }
            else
            {
                // Detach the subgraph from vertex closedWalk[0]; the subgraph
                // is attached via a filament.
                Vertex* current = closedWalk[0];
                Vertex* next = closedWalk[1];

                auto clone = std::make_shared<Vertex>(current->index, current->position);
                mVertices.push_back(clone);

                Remove(next, current);
                Insert(next, clone.get());

                // Get the subgraph (it is a single connected component).
                std::vector<Vertex*> component{};
                DepthFirstSearch(clone.get(), component);

                // Extract the cycles of the subgraph.
                tree->children.push_back(ExtractBasis(component));
                if (tree->cycle.size() == 0 && tree->children.size() == 1)
                {
                    // Replace the parent by the child to avoid having two
                    // empty cycles in parent/child.
                    auto child = tree->children.back();
                    tree->cycle = std::move(child->cycle);
                    tree->children = std::move(child->children);
                }
            }

            return tree;
        }

        std::vector<IndexType> ExtractCycle(std::vector<Vertex*>& closedWalk)
        {
            // The logic of this function was designed not to remove
            // filaments after the cycle deletion is complete. This is an
            // iterative process that removes polylines that occur after
            // a cycle has been removed, causing part or all of a cycle
            // boundary to appear to be a filament for the modified graph.

            // The closed walk is a cycle.
            std::vector<IndexType> cycle(closedWalk.size());
            for (size_t i = 0; i < closedWalk.size(); ++i)
            {
                cycle[i] = closedWalk[i]->index;
            }

            // The clockwise-most edge is always removable.
            Vertex* vertex0 = closedWalk[0];
            Vertex* vertex1 = closedWalk[1];
            Vertex* vBranch = (vertex0->adjacents.size() > 2 ? vertex0 : nullptr);
            Remove(vertex0, vertex1);

            // Remove edges while traversing counterclockwise.
            while (vertex1 != vBranch && vertex1->adjacents.size() == 1)
            {
                Vertex* adj = *vertex1->adjacents.begin();
                Remove(adj, vertex1);
                vertex1 = adj;
            }

            if (vertex1 != vertex0)
            {
                // If vertex1 had exactly 3 adjacent vertices, removal of the
                // CCW edge that shared vertex1 leads to vertex1 having 2
                // adjacent vertices. When the CW removal occurs and we reach
                // vertex1, the edge deletion will lead to vertex1 having 1
                // adjacent vertex, making it a filament endpoint.  We must
                // ensure we do not delete vertex1 in this case, allowing the
                // recursive algorithm to handle the filament later.
                vBranch = vertex1;

                // Remove edges while traversing clockwise.
                while (vertex0 != vBranch && vertex0->adjacents.size() == 1)
                {
                    vertex1 = *vertex0->adjacents.begin();
                    Remove(vertex0, vertex1);
                    vertex0 = vertex1;
                }
            }
            // else the cycle is its own connected component.

            return cycle;
        }

        Vertex* GetClockwiseMost(Vertex* vPrev, Vertex* vCurr) const
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            RPosition rDCurr{}, rDNext{};
            if (vPrev)
            {
                rDCurr = Sub(vCurr->GetRPosition(), vPrev->GetRPosition());
            }
            else
            {
                rDCurr = RPosition{ Rational(0), Rational(-1) };
            }

            for (auto vAdj : vCurr->adjacents)
            {
                // vAdj is a vertex adjacent to vCurr. No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                RPosition rDAdj = Sub(vAdj->GetRPosition(), vCurr->GetRPosition());

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    rDNext = rDAdj;
                    vCurrConvex = (GetSignDet(rDNext, rDCurr) <= 0);
                    continue;
                }

                // Update if the next candidate is clockwise of the current
                // clockwise-most vertex.
                int32_t signDet0 = GetSignDet(rDCurr, rDAdj);
                int32_t signDet1 = GetSignDet(rDNext, rDAdj);
                if (vCurrConvex)
                {
                    if (signDet0 < 0 || signDet1 < 0)
                    {
                        vNext = vAdj;
                        rDNext = rDAdj;
                        vCurrConvex = (GetSignDet(rDNext, rDCurr) <= 0);
                    }
                }
                else
                {
                    if (signDet0 < 0 && signDet1 < 0)
                    {
                        vNext = vAdj;
                        rDNext = rDAdj;
                        vCurrConvex = (GetSignDet(rDNext, rDCurr) < 0);
                    }
                }
            }

            return vNext;
        }

        Vertex* GetCounterclockwiseMost(Vertex* vPrev, Vertex* vCurr) const
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            RPosition rDCurr{}, rDNext{};
            if (vPrev)
            {
                rDCurr = Sub(vCurr->GetRPosition(), vPrev->GetRPosition());
            }
            else
            {
                rDCurr = RPosition{ Rational(0), Rational(-1) };
            }

            for (auto vAdj : vCurr->adjacents)
            {
                // vAdj is a vertex adjacent to vCurr. No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                RPosition rDAdj = Sub(vAdj->GetRPosition(), vCurr->GetRPosition());

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    rDNext = rDAdj;
                    vCurrConvex = (GetSignDet(rDNext, rDCurr) <= 0);
                    continue;
                }

                // Select the next candidate if it is counterclockwise of the
                // current counterclockwise-most vertex.
                int32_t signDet0 = GetSignDet(rDCurr, rDAdj);
                int32_t signDet1 = GetSignDet(rDNext, rDAdj);
                if (vCurrConvex)
                {
                    if (signDet0 > 0 && signDet1 > 0)
                    {
                        vNext = vAdj;
                        rDNext = rDAdj;
                        vCurrConvex = (GetSignDet(rDNext, rDCurr) <= 0);
                    }
                }
                else
                {
                    if (signDet0 > 0 || signDet1 > 0)
                    {
                        vNext = vAdj;
                        rDNext = rDAdj;
                        vCurrConvex = (GetSignDet(rDNext, rDCurr) <= 0);
                    }
                }
            }

            return vNext;
        }

        // Storage for vertices of the original graph and for new vertices
        // added during graph traversal.
        std::vector<std::shared_ptr<Vertex>> mVertices;

        // The output of the Extract function call.
        std::vector<IndexType> mIsolatedVertices;
        std::vector<Filament> mFilaments;
        Forest mForest;
    };
}
