// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.12.24

#pragma once

// Extract the minimal cycle basis for a planar graph. The input vertices and
// edges must form a graph for which edges intersect only at vertices; that
// is, no two edges must intersect at an interior point of one of the edges.
// The algorithm is described in 
//   https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf
// The graph might have filaments, which are polylines in the graph that are
// not shared by a cycle. These are also extracted by the implementation.
// Because the inputs to the constructor are vertices and edges of the graph,
// isolated vertices are ignored.
//
// The computations that determine which adjacent vertex to visit next during
// a filament or cycle traversal do not require division, so the exact
// arithmetic type BSNumber<UIntegerAP32> suffices for ComputeType when you
// want to ensure a correct output. Floating-point rounding errors
// potentially can lead to an incorrect output.

#include <Mathematics/MinHeap.h>
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
        {
            static_assert(std::is_integral<IndexType>::value && sizeof(IndexType) >= 2,
                "IndexType must be a signed or unsigned integer type of size at least 2 bytes.");
        }

        // Disallow copy semantics.
        MinimalCycleBasis(MinimalCycleBasis const&) = delete;
        MinimalCycleBasis& operator=(MinimalCycleBasis const&) = delete;

        // Disallow move semantics.
        MinimalCycleBasis(MinimalCycleBasis&&) = delete;
        MinimalCycleBasis& operator=(MinimalCycleBasis&&) = delete;

        // Extract the cycles and filaments.
        static void Extract(
            std::vector<Position> const& positions,
            std::vector<Edge> const& edges,
            Forest& forest,
            std::vector<Filament>& filaments)
        {
            forest.clear();
            filaments.clear();
            if (positions.size() == 0 || edges.size() == 0)
            {
                // The graph is empty, so there are no filaments or cycles.
                return;
            }

            // Determine the unique positions referenced by the edges.
            std::map<IndexType, std::shared_ptr<Vertex>> unique{};
            for (auto const& edge : edges)
            {
                for (size_t i = 0; i < 2; ++i)
                {
                    IndexType name = edge[i];
                    if (unique.find(name) == unique.end())
                    {
                        auto vertex = std::make_shared<Vertex>(name, &positions[name]);
                        unique.insert(std::make_pair(name, vertex));
                    }
                }
            }

            // The vertexStorage[] array has ownership of the Vertex objects.
            // The vertices[] store raw pointers to these objects.
            std::vector<std::shared_ptr<Vertex>> vertexStorage{};
            std::vector<Vertex*> vertices{};
            vertexStorage.reserve(unique.size());
            vertices.reserve(unique.size());
            for (auto const& element : unique)
            {
                vertexStorage.push_back(element.second);
                vertices.push_back(element.second.get());
            }

            // Determine the adjacencies from the edge information.
            for (auto const& edge : edges)
            {
                // Both iter0 and iter1 cannot equal unique.end() because
                // the edges were inserted in a previous block of this
                // function.
                auto iter0 = unique.find(edge[0]);
                auto iter1 = unique.find(edge[1]);
                iter0->second->adjacent.insert(iter1->second.get());
                iter1->second->adjacent.insert(iter0->second.get());
            }

            // Get the connected components of the graph. The 'visited' flags
            // are 0 (unvisited), 1 (discovered), 2 (finished). The Vertex
            // constructor sets 'visited' to 0.
            std::vector<std::vector<Vertex*>> components{};
            for (auto vInitial : vertices)
            {
                if (vInitial->visited == 0)
                {
                    components.push_back(std::vector<Vertex*>{});
                    DepthFirstSearch(vInitial, components.back());
                }
            }

            // The depth-first search is used later for collecting vertices
            // for subgraphs that are detached from the main graph, so the
            // 'visited' flags must be reset to zero after component finding.
            for (auto vertex : vertices)
            {
                vertex->visited = 0;
            }

            // Get the primitives for the components. The filaments are not
            // removed from the graph by the function GetFilaments(...)
            // because ExtractBasis(...) relies on their existence. The
            // ExtractBasis(...) function will remove the filaments from the
            // graph and discard them.
            for (auto& component : components)
            {
                GetFilaments(component, filaments);

                auto tree = ExtractBasis(component, vertexStorage);
                if (tree->children.size() > 0 || tree->cycle.size() > 0)
                {
                    forest.push_back(tree);
                }
            }
        }

    private:
        struct Vertex
        {
            Vertex()
                :
                name(0),
                position(nullptr),
                adjacent{},
                visited(0)
            {
            }

            Vertex(IndexType inName, Position const* inPosition)
                :
                name(inName),
                position(inPosition),
                adjacent{},
                visited(0)
            {
            }

            bool operator< (Vertex const& vertex) const
            {
                return name < vertex.name;
            }

            // The index into the 'positions' input provided to the call to
            // Extract(...). The index is used when reporting cycles to the
            // caller of Extract(...).
            IndexType name;

            // Multiple vertices can share a position during processing of
            // graph components.
            Position const* position;

            // The vertexStorage local std::vector declared in Extract(...)
            // owns the Vertex objects and maintains the reference counts on
            // those objects. The adjacent pointers are considered to be weak
            // pointers, but neither object ownership nor reference counting
            // are required by 'adjacent'.
            std::set<Vertex*> adjacent;

            // Support for depth-first traversal of a graph.
            uint32_t visited;
        };

        // Extract(...) uses GetComponents(...) and DepthFirstSearch(...) to
        // compute the connected components of the graph implied by the input
        // 'edges'. Recursive processing uses only DepthFirstSearch(...) to
        // collect vertices of the subgraphs of the original graph.
        static void DepthFirstSearch(Vertex* vInitial, std::vector<Vertex*>& component)
        {
            std::stack<Vertex*> vStack{};
            vStack.push(vInitial);
            while (vStack.size() > 0)
            {
                Vertex* vertex = vStack.top();
                vertex->visited = 1;
                size_t i = 0;
                for (auto adjacent : vertex->adjacent)
                {
                    if (adjacent && adjacent->visited == 0)
                    {
                        vStack.push(adjacent);
                        break;
                    }
                    ++i;
                }

                if (i == vertex->adjacent.size())
                {
                    vertex->visited = 2;
                    component.push_back(vertex);
                    vStack.pop();
                }
            }
        }

        // Support for traversing a simply connected component of the graph.
        static std::shared_ptr<Tree> ExtractBasis(
            std::vector<Vertex*>& component,
            std::vector<std::shared_ptr<Vertex>>& vertexStorage)
        {
            // The root will not have its 'cycle' member set. The children are
            // the cycle trees extracted from the component.
            auto tree = std::make_shared<Tree>();
            while (component.size() > 0)
            {
                RemoveFilaments(component);
                if (component.size() > 0)
                {
                    tree->children.push_back(
                        ExtractCycleFromComponent(component, vertexStorage));
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

        static void GetFilaments(std::vector<Vertex*>& component,
            std::vector<Filament>& filaments)
        {
            // Locate all filament endpoints, which are vertices with each
            // having exactly one adjacent vertex.
            std::vector<Vertex*> endpoints{};
            for (auto vertex : component)
            {
                if (vertex->adjacent.size() == 1)
                {
                    endpoints.push_back(vertex);
                }
            }

            if (endpoints.size() > 0)
            {
                Filament filament{};

                // Traverse the filament starting at an endpoint. The other
                // endpoint is marked as 'visited' if it has only one adjacent
                // vertex; that is, it is not a branch point of the graph.
                for (auto vertex : endpoints)
                {
                    if (!vertex->visited)
                    {
                        // The current vertex is guaranteed to have 1 adjacent
                        // vertex because it is an endpoint.
                        Vertex* current = vertex;
                        filament.push_back(current->name);

                        Vertex* next = *vertex->adjacent.begin();
                        filament.push_back(next->name);
                        while (next->adjacent.size() == 2)
                        {
                            // The next vertex has 2 adjacent vertices. One
                            // of them is the current vertex. The traversal
                            // should continue with the other adjacent vertex.
                            auto iter = next->adjacent.begin();
                            if (*iter == current)
                            {
                                ++iter;
                            }
                            current = next;
                            next = *iter;
                            filament.push_back(next->name);
                        }

                        // At this time, the next vertex is the other endpoint
                        // of the filament. It has 3 or more adjacent vertices
                        // (a branch point of the graph) or 1 adjacent vertex
                        // (an endpoint). If an endpoint, mark it as visited
                        // so that the filament is not traversed in the
                        // opposite direction to form another (already
                        // visited) filament.
                        if (next->adjacent.size() == 1)
                        {
                            next->visited = 1;
                        }
                    }
                }

                // Restore the visited flags because they are used later by
                // DepthFirstSearch(...).
                for (auto vertex : endpoints)
                {
                    vertex->visited = 0;
                }

                filaments.emplace_back(filament);
            }
        }

        static void RemoveFilaments(std::vector<Vertex*>& component)
        {
            // Locate all filament endpoints, which are vertices with each
            // having exactly one adjacent vertex.
            std::vector<Vertex*> endpoints{};
            for (auto vertex : component)
            {
                if (vertex->adjacent.size() == 1)
                {
                    endpoints.push_back(vertex);
                }
            }

            if (endpoints.size() > 0)
            {
                // Remove the filaments from the component. If a filament has
                // two endpoints, each having one adjacent vertex, the
                // adjacency set of the final visited vertex becomes empty.
                // This condition must be tested before starting a new
                // filament removal.
                for (auto vertex : endpoints)
                {
                    if (vertex->adjacent.size() == 1)
                    {
                        // Traverse the filament and remove the vertices.
                        while (vertex->adjacent.size() == 1)
                        {
                            // Break the connection between the two vertices.
                            Vertex* adjacent = *vertex->adjacent.begin();
                            adjacent->adjacent.erase(vertex);
                            vertex->adjacent.erase(adjacent);

                            // Traverse to the adjacent vertex.
                            vertex = adjacent;
                        }
                    }
                }

                // At this time the component is either empty because it was
                // an open polyline or it has no filaments and at least one
                // cycle. Remove the isolated vertices generated by filament
                // extraction.
                std::vector<Vertex*> remaining{};
                remaining.reserve(component.size());
                for (auto vertex : component)
                {
                    if (vertex->adjacent.size() > 0)
                    {
                        remaining.push_back(vertex);
                    }
                }
                component = std::move(remaining);
            }
        }

        static std::shared_ptr<Tree> ExtractCycleFromComponent(
            std::vector<Vertex*>& component,
            std::vector<std::shared_ptr<Vertex>>& vertexStorage)
        {
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
            auto tree = ExtractCycleFromClosedWalk(closedWalk, vertexStorage);

            // The isolated vertices generated by cycle removal are also
            // removed from the component.
            std::vector<Vertex*> remaining{};
            remaining.reserve(component.size());
            for (auto vertex : component)
            {
                if (vertex->adjacent.size() > 0)
                {
                    remaining.push_back(vertex);
                }
            }
            component = std::move(remaining);

            return tree;
        }

        static std::shared_ptr<Tree> ExtractCycleFromClosedWalk(
            std::vector<Vertex*>& closedWalk,
            std::vector<std::shared_ptr<Vertex>>& vertexStorage)
        {
            auto tree = std::make_shared<Tree>();

            std::map<Vertex*, size_t> duplicates{};
            std::set<size_t> detachments{};
            size_t numClosedWalk = closedWalk.size();
            for (size_t i = 1; i + 1 < numClosedWalk; ++i)
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
                size_t iMin = diter->second;
                size_t iMax = i;
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
                // <closedWalk[0],closedWalk[1]>. However, this test must be
                // executed even for the known detachment points. The ensuing
                // logic is designed to handle this and reduce the amount of
                // code, so insert closedWalk[0] into the detachment set and
                // ignore it later if it actually is not.
                detachments.insert(0);

                // Detach subgraphs from the vertices of the cycle.
                for (auto i : detachments)
                {
                    Vertex* original = closedWalk[i];
                    Vertex* maxVertex = closedWalk[i + 1];
                    Vertex* minVertex = (i > 0 ? closedWalk[i - 1] : closedWalk[numClosedWalk - 2]);

                    Position dMin{}, dMax{};
                    for (size_t j = 0; j < 2; ++j)
                    {
                        dMin[j] = (*minVertex->position)[j] - (*original->position)[j];
                        dMax[j] = (*maxVertex->position)[j] - (*original->position)[j];
                    }

                    bool isConvex = (dMax[0] * dMin[1] >= dMax[1] * dMin[0]);
                    std::set<Vertex*> inWedge{};
                    std::set<Vertex*> adjacent = original->adjacent;
                    for (auto vertex : adjacent)
                    {
                        if (vertex->name == minVertex->name || vertex->name == maxVertex->name)
                        {
                            continue;
                        }

                        Position dVer{};
                        for (size_t j = 0; j < 2; ++j)
                        {
                            dVer[j] = (*vertex->position)[j] - (*original->position)[j];
                        }

                        bool containsVertex{};
                        if (isConvex)
                        {
                            containsVertex =
                                dVer[0] * dMin[1] > dVer[1] * dMin[0] &&
                                dVer[0] * dMax[1] < dVer[1] * dMax[0];
                        }
                        else
                        {
                            containsVertex =
                                (dVer[0] * dMin[1] > dVer[1] * dMin[0]) ||
                                (dVer[0] * dMax[1] < dVer[1] * dMax[0]);
                        }

                        if (containsVertex)
                        {
                            inWedge.insert(vertex);
                        }
                    }

                    if (inWedge.size() > 0)
                    {
                        // The clone will manage the adjacents for 'original'
                        // that lie inside the wedge defined by the first and
                        // last edges of the subgraph rooted at 'original'.
                        // The sorting is in the clockwise direction.
                        auto clone = std::make_shared<Vertex>(original->name, original->position);
                        vertexStorage.push_back(clone);

                        // Detach the edges inside the wedge.
                        for (auto vertex : inWedge)
                        {
                            original->adjacent.erase(vertex);
                            vertex->adjacent.erase(original);
                            clone->adjacent.insert(vertex);
                            vertex->adjacent.insert(clone.get());
                        }

                        // Get the subgraph (it is a single connected
                        // component).
                        std::vector<Vertex*> component{};
                        DepthFirstSearch(clone.get(), component);

                        // Extract the cycles of the subgraph.
                        tree->children.push_back(ExtractBasis(component, vertexStorage));
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
                Vertex* original = closedWalk[0];
                Vertex* adjacent = closedWalk[1];

                auto clone = std::make_shared<Vertex>(original->name, original->position);
                vertexStorage.push_back(clone);

                original->adjacent.erase(adjacent);
                adjacent->adjacent.erase(original);
                clone->adjacent.insert(adjacent);
                adjacent->adjacent.insert(clone.get());

                // Get the subgraph (it is a single connected component).
                std::vector<Vertex*> component{};
                DepthFirstSearch(clone.get(), component);

                // Extract the cycles of the subgraph.
                tree->children.push_back(ExtractBasis(component, vertexStorage));
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

        static std::vector<IndexType> ExtractCycle(std::vector<Vertex*>& closedWalk)
        {
            // The logic of this function was designed not to remove filaments
            // after the cycle deletion is complete. This is an iterative
            // process that removes polylines that occur *after* a cycle has
            // been removed, causing part or all of a cycle boundary to appear
            // to be a filament for the *modified* graph.

            // The closed walk is a cycle.
            std::vector<IndexType> cycle(closedWalk.size());
            for (size_t i = 0; i < closedWalk.size(); ++i)
            {
                cycle[i] = closedWalk[i]->name;
            }

            // The clockwise-most edge is always removable.
            Vertex* v0 = closedWalk[0];
            Vertex* v1 = closedWalk[1];
            Vertex* vBranch = (v0->adjacent.size() > 2 ? v0 : nullptr);
            v0->adjacent.erase(v1);
            v1->adjacent.erase(v0);

            // Remove edges while traversing counterclockwise.
            while (v1 != vBranch && v1->adjacent.size() == 1)
            {
                Vertex* adj = *v1->adjacent.begin();
                v1->adjacent.erase(adj);
                adj->adjacent.erase(v1);
                v1 = adj;
            }

            if (v1 != v0)
            {
                // If v1 had exactly 3 adjacent vertices, removal of the
                // counterclockwise edge that shared v1 leads to v1 having 2
                // adjacent vertices. When the clockwise removal occurs and
                // v1 is reached, the edge deletion will lead to v1 having 1
                // adjacent vertex, making it a filament endpoint. Ensure that
                // v1 is not deleted in this case, allowing the recursive
                // algorithm to handle the filament later.
                vBranch = v1;

                // Remove edges while traversing clockwise.
                while (v0 != vBranch && v0->adjacent.size() == 1)
                {
                    v1 = *v0->adjacent.begin();
                    v0->adjacent.erase(v1);
                    v1->adjacent.erase(v0);
                    v0 = v1;
                }
            }
            // else the cycle is its own connected component.

            return cycle;
        }

        static Vertex* GetClockwiseMost(Vertex* vPrev, Vertex* vCurr)
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            Position dCurr{}, dNext{};
            if (vPrev)
            {
                dCurr[0] = (*vCurr->position)[0] - (*vPrev->position)[0];
                dCurr[1] = (*vCurr->position)[1] - (*vPrev->position)[1];
            }
            else
            {
                dCurr[0] = static_cast<T>(0);
                dCurr[1] = static_cast<T>(-1);
            }

            for (auto vAdj : vCurr->adjacent)
            {
                // vAdj is a vertex adjacent to vCurr. No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                Position dAdj{};
                dAdj[0] = (*vAdj->position)[0] - (*vCurr->position)[0];
                dAdj[1] = (*vAdj->position)[1] - (*vCurr->position)[1];

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    dNext = dAdj;
                    vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    continue;
                }

                // Update if the next candidate is clockwise of the current
                // clockwise-most vertex.
                if (vCurrConvex)
                {
                    if (dCurr[0] * dAdj[1] < dCurr[1] * dAdj[0] ||
                        dNext[0] * dAdj[1] < dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
                else
                {
                    if (dCurr[0] * dAdj[1] < dCurr[1] * dAdj[0] &&
                        dNext[0] * dAdj[1] < dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] < dNext[1] * dCurr[0]);
                    }
                }
            }

            return vNext;
        }

        static Vertex* GetCounterclockwiseMost(Vertex* vPrev, Vertex* vCurr)
        {
            Vertex* vNext = nullptr;
            bool vCurrConvex = false;
            Position dCurr{}, dNext{};
            if (vPrev)
            {
                dCurr[0] = (*vCurr->position)[0] - (*vPrev->position)[0];
                dCurr[1] = (*vCurr->position)[1] - (*vPrev->position)[1];
            }
            else
            {
                dCurr[0] = static_cast<T>(0);
                dCurr[1] = static_cast<T>(-1);
            }

            for (auto vAdj : vCurr->adjacent)
            {
                // vAdj is a vertex adjacent to vCurr. No backtracking is
                // allowed.
                if (vAdj == vPrev)
                {
                    continue;
                }

                // Compute the potential direction to move in.
                Position dAdj{};
                dAdj[0] = (*vAdj->position)[0] - (*vCurr->position)[0];
                dAdj[1] = (*vAdj->position)[1] - (*vCurr->position)[1];

                // Select the first candidate.
                if (!vNext)
                {
                    vNext = vAdj;
                    dNext = dAdj;
                    vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    continue;
                }

                // Select the next candidate if it is counterclockwise of the
                // current counterclockwise-most vertex.
                if (vCurrConvex)
                {
                    if (dCurr[0] * dAdj[1] > dCurr[1] * dAdj[0] &&
                        dNext[0] * dAdj[1] > dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
                else
                {
                    if (dCurr[0] * dAdj[1] > dCurr[1] * dAdj[0] ||
                        dNext[0] * dAdj[1] > dNext[1] * dAdj[0])
                    {
                        vNext = vAdj;
                        dNext = dAdj;
                        vCurrConvex = (dNext[0] * dCurr[1] <= dNext[1] * dCurr[0]);
                    }
                }
            }

            return vNext;
        }
    };
}
