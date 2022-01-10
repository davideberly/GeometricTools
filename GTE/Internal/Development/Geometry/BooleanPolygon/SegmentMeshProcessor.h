// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/AlignedBox.h>
#include <Mathematics/Vector2.h>
#include <vector>

namespace gte
{
    template <typename Real>
    class SegmentMeshProcessor
    {
    public:
        // The segment mesh consists of a collection of 2D positions and
        // a collection of directed edges.
        using Vertex = Vector2<Real>;
        using Edge = std::array<size_t, 2>;

        SegmentMeshProcessor(std::vector<Vertex> const& vertices,
            std::vector<Edge> const& edges)
            :
            mVertices(vertices),
            mEdges(edges),
            mDegenerateEdges{},
            mOutOfRangeEdges{},
            mDuplicateVertices{},
            mUnusedVertices{}
        {
            LogAssert(vertices.size() > 0 && edges.size() > 0, "Invalid argument.");

            RemoveDegenerateEdges();
            RemoveOutOfRangeEdges();
            RemoveDuplicateVertices();
            RemoveUnusedVertices();
        }

    private:
        // Degenerate edges are of the form <v,v>.
        void RemoveDegenerateEdges()
        {
            std::vector<Edge> outEdges;
            outEdges.reserve(mEdges.size());
            for (auto const& edge : mEdges)
            {
                if (edge[0] != edge[1])
                {
                    outEdges.push_back(edge);
                }
                else
                {
                    // STATISTICS
                    mDegenerateEdges.push_back(edge);
                }
            }
            mEdges = std::move(outEdges);
        }

        // Out-of-range edges <v[0],v[1]> are those for which one of v[i] is
        // out of range; that is v[i] >= mVertices.size().
        void RemoveOutOfRangeEdges()
        {
            std::vector<Edge> outEdges;
            outEdges.reserve(mEdges.size());
            for (auto const& edge : mEdges)
            {
                if (edge[0] < mVertices.size() && edge[1] < mVertices.size())
                {
                    outEdges.push_back(edge);
                }
                else
                {
                    // STATISTICS
                    mOutOfRangeEdges.push_back(edge);
                }
            }
            mEdges = std::move(outEdges);
        }

        void RemoveDuplicateVertices()
        {
            // Construct the unique vertices.
            size_t const numInVertices = mVertices.size();
            std::vector<size_t> inToOutMapping(numInVertices);
            std::set<size_t> duplicates;
            size_t numOutVertices = 0;
            std::map<Vertex, size_t> vmap;
            for (size_t v = 0; v < numInVertices; ++v)
            {
                auto const iter = vmap.find(mVertices[v]);
                if (iter != vmap.end())
                {
                    // The vertex is a duplicate of one inserted earlier into
                    // the map. Its index v will be modified to that of the
                    // first-found vertex.
                    inToOutMapping[v] = iter->second;
                    duplicates.insert(v);
                }
                else
                {
                    // The vertex occurs for the first time.
                    vmap.insert(std::make_pair(mVertices[v], numOutVertices));
                    inToOutMapping[v] = numOutVertices;
                    ++numOutVertices;
                }
            }

            if (duplicates.size() == 0)
            {
                // All vertices are unique. There is no need to repackage the
                // vertices and edges.
                return;
            }

            // STATISTICS
            mDuplicateVertices.reserve(duplicates.size());
            for (auto v : duplicates)
            {
                mDuplicateVertices.push_back(mVertices[v]);
            }

            // Pack the unique vertices into an array.
            std::vector<Vertex> outVertices(numOutVertices);
            for (auto const& element : vmap)
            {
                outVertices[element.second] = element.first;
            }
            mVertices = std::move(outVertices);

            // Re-index the edges to account for the removal of duplicate
            // vertices.
            std::vector<Edge> outEdges(mEdges.size());
            for (size_t e = 0; e < mEdges.size(); ++e)
            {
                auto const& edge = mEdges[e];
                outEdges[e][0] = inToOutMapping[edge[0]];
                outEdges[e][1] = inToOutMapping[edge[1]];
            }
            mEdges = std::move(outEdges);
        }

        void RemoveUnusedVertices()
        {
            // Get the unique set of used indices.
            std::set<size_t> usedIndices;
            for (auto const& edge : mEdges)
            {
                usedIndices.insert(edge[0]);
                usedIndices.insert(edge[1]);
            }

            if (usedIndices.size() != mVertices.size())
            {
                // STATISTICS
                mUnusedVertices.reserve(mVertices.size() - usedIndices.size());
                for (size_t v = 0; v < mVertices.size(); ++v)
                {
                    auto iter = usedIndices.find(v);
                    if (usedIndices.find(v) == usedIndices.end())
                    {
                        mUnusedVertices.push_back(mVertices[v]);
                    }
                }

                // Locate the used vertices and pack them into an array.
                std::vector<Vertex> outVertices(usedIndices.size());
                size_t numOutVertices = 0;
                std::map<size_t, size_t> vmap;
                for (auto oldIndex : usedIndices)
                {
                    outVertices[numOutVertices] = mVertices[oldIndex];
                    vmap.insert(std::make_pair(oldIndex, numOutVertices));
                    ++numOutVertices;
                }
                mVertices = std::move(outVertices);

                // Reassign the old indices to the new indices.
                std::vector<Edge> outEdges(mEdges.size());
                for (size_t e = 0; e < mEdges.size(); ++e)
                {
                    auto const& edge = mEdges[e];
                    outEdges[e][0] = vmap.find(edge[0])->second;
                    outEdges[e][1] = vmap.find(edge[1])->second;
                }
                mEdges = std::move(outEdges);
            }
        }

    private:
        // The fully processed mesh vertices and edges.
        std::vector<Vertex> mVertices;
        std::vector<Edge> mEdges;

        // Statistics about the mesh processing.
        std::vector<Edge> mDegenerateEdges;
        std::vector<Edge> mOutOfRangeEdges;
        std::vector<Vertex> mDuplicateVertices;
        std::vector<Vertex> mUnusedVertices;
    };
}
