// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2023.02.02

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Vector3.h>
#include <Mathematics/EdgeKey.h>
#include <Mathematics/TriangleKey.h>
#include <Mathematics/MinHeap.h>
#include <Graphics/CLODCollapseRecord.h>
#include <limits>
#include <map>
#include <set>

// Given a triangle mesh, CLODMeshCreator generates an array of collapse
// records. Each record represents an incremental change in the mesh and is
// used for level of detail.

namespace gte
{
    // The VertexAtom is the structure that represents an element of the
    // graphics vertex buffer that is associated with the mesh to be
    // decimated. It must be a POD (plain old data) structure. This structure
    // must have a 3-tuple float position that is accessed by
    //   Vector3<float> VertexAtom::GetPosition() const;
    // An object is returned rather than a const-reference to allow the caller
    // to assemble the position into a 3-tuple if it is not naturally stored
    // that way.

    template <typename VertexAtom>
    class CLODMeshCreator
    {
    public:
        CLODMeshCreator()
            :
            mVertexAtoms{},
            mIndices{},
            mVertices{},
            mEdges{},
            mTriangles{},
            mNumTriangles(0),
            mHeap{},
            mCollapses{},
            mVerticesRemaining{},
            mTrianglesRemaining{}
        {
        }

        ~CLODMeshCreator() = default;

        void operator()(
            std::vector<VertexAtom> const& inVertexAtoms,
            std::vector<int32_t> const& inIndices,
            std::vector<VertexAtom>& outVertexAtoms,
            std::vector<int32_t>& outIndices,
            std::vector<CLODCollapseRecord>& records)
        {
            LogAssert(
                inIndices.size() % 3 == 0,
                "Invalid number of indices.");

            outVertexAtoms.clear();
            outIndices.clear();
            records.clear();

            mVertexAtoms = inVertexAtoms;
            mIndices = inIndices;
            mNumTriangles = static_cast<int32_t>(mIndices.size() / 3);
            mVertices.resize(mVertexAtoms.size());
            mHeap.Reset(static_cast<int32_t>(mIndices.size()));

            // Ensure the vertex and index buffers are valid for edge
            // collapsing. If they are not, an exception is thrown.
            ValidateBuffers();

            // Create the vertex-edge-triangle graph.
            int32_t const* currentIndex = mIndices.data();
            for (int32_t t = 0; t < mNumTriangles; ++t)
            {
                int32_t v0 = *currentIndex++;
                int32_t v1 = *currentIndex++;
                int32_t v2 = *currentIndex++;
                InsertTriangle(TriangleKey<true>(v0, v1, v2), t);
            }

            // Vertices that are endpoints of boundary edges, or are
            // nonmanifold in that they are shared by two edge-triangle
            // connected compoments, cannot be allowed to collapse.
            ClassifyCollapsibleVertices();

            // Update the heap of edges.
            for (auto const& element : mEdges)
            {
                LogAssert(
                    element.second.record->index < mHeap.GetNumElements(),
                    "Unexpected condition.");

                mHeap.Update(element.second.record, ComputeMetric(element.first));
            }

            while (mHeap.GetNumElements() > 0)
            {
                EdgeKey<false> eKey{};
                float metric{};
                mHeap.GetMinimum(eKey, metric);
                if (metric == std::numeric_limits<float>::max())
                {
                    // All remaining heap elements have infinite metrics.
                    // Validate the results and throw an exception if not
                    // valid.
                    ValidateResults();
                    break;
                }

                int32_t indexThrow = CanCollapse(eKey);
                if (indexThrow >= 0)
                {
                    Collapse(eKey, indexThrow);
                }
                else
                {
                    auto emIter = mEdges.find(eKey);
                    LogAssert(
                        emIter->second.record->index < mHeap.GetNumElements(),
                        "Unexpected condition.");

                    mHeap.Update(emIter->second.record, std::numeric_limits<float>::max());
                }
            }

            // Reorder the vertex buffer so that the vertices are listed in
            // decreasing time of removal by edge collapses. For example, the
            // first vertex to be removed during an edge collapse is the last
            // vertex in the buffer. Reorder the index buffer so that the
            // triangles are listed in decreasing time of removal by edge
            // collapses. For example, the first pair of triangles to be
            // removed during an edge collapse are the last triangles in the
            // buffer.
            ReorderBuffers();

            // The collapse records store the incremental changes that are
            // used for dynamic LOD changes.
            ComputeRecords(records);

            outVertexAtoms = mVertexAtoms;
            outIndices = mIndices;
        }

    private:
        // Vertex-edge-triangle graph.
        typedef std::set<TriangleKey<true>> TriangleKeySet;
        typedef std::set<EdgeKey<false>> EdgeKeySet;

        class Vertex
        {
        public:
            Vertex()
                :
                adjEdges{},
                adjTriangles{},
                collapsible(true)
            {
            }

            EdgeKeySet adjEdges;
            TriangleKeySet adjTriangles;
            bool collapsible;
        };

        class Edge
        {
        public:
            Edge()
                :
                adjTriangles{},
                record(nullptr)
            {
            }

            TriangleKeySet adjTriangles;
            MinHeap<EdgeKey<false>, float>::Record* record;
        };

        using Triangle = int32_t;
        typedef std::vector<Vertex> VertexArray;
        typedef std::map<EdgeKey<false>, Edge> EdgeMap;
        typedef std::map<TriangleKey<true>, Triangle> TriangleMap;

        // Information about the edge collapse.
        class CollapseInfo
        {
        public:
            CollapseInfo(int32_t inVKeep = -1, int32_t inVThrow = -1)
                :
                vKeep(inVKeep),
                vThrow(inVThrow),
                tThrow0(-1),
                tThrow1(-1)
            {
            }

            int32_t vKeep, vThrow, tThrow0, tThrow1;
        };


        void ValidateBuffers() const
        {
            TriangleKeySet triangles{};
            std::set<int32_t> vertexIndices{};
            int32_t const* currentIndex = mIndices.data();
            for (int32_t t = 0; t < mNumTriangles; ++t)
            {
                int32_t v0 = *currentIndex++;
                int32_t v1 = *currentIndex++;
                int32_t v2 = *currentIndex++;

                // For now, the input should be from triangle meshes or fans.
                // The edge collapse algorithm must be modified to deal with
                // triangle strips for which degenerate triangles were added
                // to produce long strips.
                LogAssert(
                    v0 != v1 && v0 != v2 && v1 != v2,
                    "Degenerate triangles not allowed.");

                vertexIndices.insert(v0);
                vertexIndices.insert(v1);
                vertexIndices.insert(v2);

                auto result = triangles.insert(TriangleKey<true>(v0, v1, v2));

                // Test whether the index buffer contains repeated triangles.
                // The edge collapse algorithm is not designed to handle
                // repeats. 
                LogAssert(
                    result.second == true,
                    "Index buffer contains repeated triangles.");
            }

            // Test whether the vertex buffer has vertices that are not
            // referenced by the index buffer. This is a problem, because the
            // vertex buffer is reordered based on the order of the edge
            // collapses. Any other index buffer that references the input
            // vertex buffer is now invalid.
            LogAssert(
                mVertices.size() <= vertexIndices.size() &&
                static_cast<int32_t>(mVertices.size()) == (*vertexIndices.rbegin() + 1),
                "Index buffer does not reference all vertices."
            );

        }

        void InsertTriangle(TriangleKey<true> const& tKey, Triangle t)
        {
            // Create the edge keys for the triangle.
            std::array<EdgeKey<false>, 3> eKey =
            {
                EdgeKey<false>(tKey.V[0], tKey.V[1]),
                EdgeKey<false>(tKey.V[1], tKey.V[2]),
                EdgeKey<false>(tKey.V[2], tKey.V[0])
            };

            // Insert each edge into its endpoints' adjacency lists.
            for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                mVertices[tKey.V[i1]].adjEdges.insert(eKey[i0]);
                mVertices[tKey.V[i1]].adjEdges.insert(eKey[i1]);
                mVertices[tKey.V[i1]].adjTriangles.insert(tKey);
            }

            for (size_t i = 0; i < 3; ++i)
            {
                auto emIter = mEdges.find(eKey[i]);
                if (emIter == mEdges.end())
                {
                    // The edge is encountered the first time. Insert it into
                    // the graph and into the heap. Insert the triangle into
                    // its adjacency list.
                    Edge& edge = mEdges[eKey[i]];
                    edge.adjTriangles.insert(tKey);
                    edge.record = mHeap.Insert(eKey[i], std::numeric_limits<float>::max());
                }
                else
                {
                    // The edge already exists in the graph. Insert the
                    // triangle into its adjacency list.
                    emIter->second.adjTriangles.insert(tKey);
                }
            }

            // Insert the triangle into the graph.
            mTriangles.insert(std::make_pair(tKey, t));
        }

        void RemoveTriangle(TriangleKey<true> const& tKey)
        {
            // Create the edge keys for the triangle.
            std::array<EdgeKey<false>, 3> eKey =
            {
                EdgeKey<false>(tKey.V[0], tKey.V[1]),
                EdgeKey<false>(tKey.V[1], tKey.V[2]),
                EdgeKey<false>(tKey.V[2], tKey.V[0])
            };

            // Remove the triangle from its vertices' adjacency lists.
            for (size_t i = 0; i < 3; ++i)
            {
                mVertices[tKey.V[i]].adjTriangles.erase(tKey);
            }

            for (size_t i0 = 2, i1 = 0; i1 < 3; i0 = i1++)
            {
                auto emIter = mEdges.find(eKey[i0]);
                LogAssert(
                    emIter != mEdges.end(),
                    "Unexpected condition.");

                emIter->second.adjTriangles.erase(tKey);
                if (emIter->second.adjTriangles.empty())
                {
                    // The edge is not shared by any triangles, so delete it
                    // from the heap.
                    LogAssert(
                        emIter->second.record->index < mHeap.GetNumElements(),
                        "Unexpected condition.");

                    mHeap.Update(emIter->second.record, -1.0f);
                    EdgeKey<false> unused{};
                    float metric{};
                    mHeap.Remove(unused, metric);
                    LogAssert(
                        metric == -1.0f,
                        "The metric should be -1.");

                    // Delete the edge from its endpoints' adjacency lists.
                    mVertices[tKey.V[i0]].adjEdges.erase(eKey[i0]);
                    mVertices[tKey.V[i1]].adjEdges.erase(eKey[i0]);

                    // Delete the edge from the graph.
                    mEdges.erase(eKey[i0]);
                }
            }

            // Remove the triangle from the graph.
            mTriangles.erase(tKey);
        }

        void ClassifyCollapsibleVertices()
        {
            // TODO: Test for nonmanifold vertices. These cannot be collapsed,
            // because they are a bridge between two locally disjoint
            // submeshes in terms of edge-triangle connectivity. The test is
            // to count the number of local connected components of
            // vertex.adjTriangles. The vertex is nonmanifold if the number of
            // components is larger than one.

            // Test the vertices to determine whether they are endpoints of
            // boundary edges of the mesh.
            for (auto& vertex : mVertices)
            {
                for (auto const& eKey : vertex.adjEdges)
                {
                    auto emIter = mEdges.find(eKey);
                    if (emIter->second.adjTriangles.size() != 2)
                    {
                        vertex.collapsible = false;
                        break;
                    }
                }
            }
        }

        float ComputeMetric(EdgeKey<false> const& eKey) const
        {
            // These weights may be adjusted to whatever you like.
            float constexpr lengthWeight = 10.0f;
            float constexpr angleWeight = 1.0f;

            // Compute the metric for the edge. Only manifold edges (exactly
            // two triangles sharing the edge) are allowed to collapse.
            auto emIter = mEdges.find(eKey);
            LogAssert(
                emIter != mEdges.end(),
                "Unexpected condition.");

            if (emIter->second.adjTriangles.size() == 2)
            {
                // Length contribution.
                Vector3<float> end0 = mVertexAtoms[eKey.V[0]].GetPosition();
                Vector3<float> end1 = mVertexAtoms[eKey.V[1]].GetPosition();
                Vector3<float> diff = end1 - end0;
                float metric = lengthWeight * Length(diff);

                // Angle/area contribution.
                TriangleKey<true> tKey = *emIter->second.adjTriangles.begin();
                Vector3<float> pos00 = mVertexAtoms[tKey.V[0]].GetPosition();
                Vector3<float> pos01 = mVertexAtoms[tKey.V[1]].GetPosition();
                Vector3<float> pos02 = mVertexAtoms[tKey.V[2]].GetPosition();
                Vector3<float> normal0 = Cross(pos01 - pos00, pos02 - pos00);

                tKey = *emIter->second.adjTriangles.rbegin();
                Vector3<float> pos10 = mVertexAtoms[tKey.V[0]].GetPosition();
                Vector3<float> pos11 = mVertexAtoms[tKey.V[1]].GetPosition();
                Vector3<float> pos12 = mVertexAtoms[tKey.V[2]].GetPosition();
                Vector3<float> normal1 = Cross(pos11 - pos10, pos12 - pos10);
                Vector3<float> cross = Cross(normal0, normal1);
                metric += angleWeight * Length(cross);

                return metric;
            }

            // Boundary edges (one triangle containing edge) and junction
            // edges (3 or more triangles sharing edge) are not allowed to
            // collapse.
            return std::numeric_limits<float>::max();
        }

        int32_t CanCollapse(EdgeKey<false> const& eKey)
        {
            // Test for collapsibility.
            int32_t indexKeep = 0, indexThrow = 0;
            if (mVertices[eKey.V[0]].collapsible)
            {
                indexKeep = 1;
                indexThrow = 0;
            }
            else if (mVertices[eKey.V[1]].collapsible)
            {
                indexKeep = 0;
                indexThrow = 1;
            }
            else
            {
                return -1;
            }

            // The collapse cannot be allowed if it leads to the mesh folding
            // over.
            int32_t vKeep = eKey.V[indexKeep];
            int32_t vThrow = eKey.V[indexThrow];
            Vector3<float> posKeep = mVertexAtoms[vKeep].GetPosition();
            Vector3<float> posThrow = mVertexAtoms[vThrow].GetPosition();

            for (auto const& tKey : mVertices[vThrow].adjTriangles)
            {
                // Compute a normal vector for the plane determined by the
                // vertices of the triangle using CCW order.
                size_t j0{};
                for (j0 = 0; j0 < 3; ++j0)
                {
                    if (tKey.V[j0] == vThrow)
                    {
                        break;
                    }
                }
                LogAssert(
                    j0 < 3,
                    "Unexpected condition.");

                size_t jm = (j0 + 2) % 3;
                size_t jp = (j0 + 1) % 3;
                Vector3<float> posM = mVertexAtoms[tKey.V[jm]].GetPosition();
                Vector3<float> posP = mVertexAtoms[tKey.V[jp]].GetPosition();
                Vector3<float> dirP = posP - posThrow;
                Vector3<float> dirM = posM - posThrow;
                Vector3<float> normalThrow = Cross(dirP, dirM);

                // Now replace the throw position by the keep position and
                // compute a normal vector for the plane determined by the
                // vertices of the triangle using CCW order.
                dirP = posP - posKeep;
                dirM = posM - posKeep;
                Vector3<float> normalKeep = Cross(dirP, dirM);

                // The collapse is not allowed when the angle between the two
                // normals is larger than 90 degrees.
                if (Dot(normalThrow, normalKeep) < 0.0f)
                {
                    return -1;
                }
            }

            return indexThrow;
        }

        void Collapse(EdgeKey<false> const& eKey, int32_t indexThrow)
        {
            // Get the endpoints of the edge to be collapsed.
            int32_t indexKeep = 1 - indexThrow;
            int32_t vKeep = eKey.V[indexKeep];
            int32_t vThrow = eKey.V[indexThrow];
            CollapseInfo collapse(vKeep, vThrow);

            // Remove all the triangles sharing the throw vertex. Create the
            // edges opposite the keep vertex for triangle insertion later.
            // The opposite edges are saved, preserving the vertex ordering.
            // This information makes it easier to determine which heap edges
            // must be updated when the new triangles are inserted into the
            // graph.
            std::set<std::array<int32_t, 3>> keepInfo{};
            TriangleKeySet needRemoval = mVertices[vThrow].adjTriangles;
            for (auto const& tKey : needRemoval)
            {
                size_t j0{};
                for (j0 = 0; j0 < 3; ++j0)
                {
                    if (tKey.V[j0] == vThrow)
                    {
                        break;
                    }
                }
                LogAssert(
                    j0 < 3,
                    "Unexpected condition.");

                auto tmIter = mTriangles.find(tKey);
                LogAssert(
                    tmIter != mTriangles.end(),
                    "Unexpected condition.");

                std::array<int32_t, 3> tuple{};
                tuple[0] = tKey.V[(j0 + 1) % 3];
                tuple[1] = tKey.V[(j0 + 2) % 3];
                tuple[2] = tmIter->second;

                if (tuple[0] != vKeep && tuple[1] != vKeep)
                {
                    keepInfo.insert(tuple);
                }
                else
                {
                    if (collapse.tThrow0 == -1)
                    {
                        collapse.tThrow0 = tuple[2];
                    }
                    else
                    {
                        LogAssert(
                            collapse.tThrow1 == -1,
                            "Unexpected condition.");

                        collapse.tThrow1 = tuple[2];
                        mCollapses.push_back(collapse);
                    }
                }

                RemoveTriangle(tKey);
            }

            // Insert the new triangles that share the keep vertex. Save the
            // edges that need to be updated in the heap.
            EdgeKeySet needUpdate{};
            for (auto const& tuple : keepInfo)
            {
                int32_t v0 = vKeep;
                int32_t v1 = tuple[0];
                int32_t v2 = tuple[1];
                Triangle t = static_cast<size_t>(tuple[2]);
                InsertTriangle(TriangleKey<true>(v0, v1, v2), t);
                needUpdate.insert(EdgeKey<false>(v0, v1));
                needUpdate.insert(EdgeKey<false>(v1, v2));
                needUpdate.insert(EdgeKey<false>(v2, v0));
            }

            // Update the heap for those edges affected by the collapse.
            for (auto const& updateKey : needUpdate)
            {
                auto emIter = mEdges.find(updateKey);
                LogAssert(
                    emIter->second.record->index < mHeap.GetNumElements(),
                    "Unexpected condition.");

                mHeap.Update(emIter->second.record, ComputeMetric(updateKey));
            }
        }

        void ValidateResults()
        {
            // Save the indices of the remaining triangles. These are needed
            // for reordering of the index buffer.
            size_t expectedNumTriangles = 2 * mCollapses.size() + mTriangles.size();
            LogAssert(
                static_cast<size_t>(mNumTriangles) == expectedNumTriangles,
                "Incorrect triangle counts."
            );

            for (auto const& element : mTriangles)
            {
                mTrianglesRemaining.push_back(element.second);
            }

            // Save the indices of the remaining vertices. These are needed
            // for reordering of the vertex buffer.
            for (size_t i = 0; i < mVertices.size(); ++i)
            {
                auto const& vertex = mVertices[i];
                bool hasEdges = (vertex.adjEdges.size() > 0);
                bool hasTriangles = (vertex.adjTriangles.size() > 0);
                LogAssert(
                    hasEdges == hasTriangles,
                    "Inconsistent edge-triangle adjacency.");

                if (vertex.adjEdges.size() > 0)
                {
                    mVerticesRemaining.push_back(static_cast<int32_t>(i));
                }
            }

            size_t expectedNumVertices = mCollapses.size() + mVerticesRemaining.size();
            LogAssert(
                mVertices.size() == expectedNumVertices,
                "Incorrect vertex counts.");
        }

        void ReorderBuffers()
        {
            // Construct the mappings between the old vertex order and the new
            // vertex order.
            int32_t const numVertices = static_cast<int32_t>(mVertexAtoms.size());
            std::vector<int32_t> vertexNewToOld(numVertices);
            std::vector<int32_t> vertexOldToNew(numVertices);
            int32_t vNew = numVertices - 1;

            for (auto const& collapse : mCollapses)
            {
                int32_t vOld = collapse.vThrow;
                vertexNewToOld[vNew] = vOld;
                vertexOldToNew[vOld] = vNew--;
            }

            for (auto const& vOld : mVerticesRemaining)
            {
                vertexNewToOld[vNew] = vOld;
                vertexOldToNew[vOld] = vNew--;
            }

            // Reorder the positions.
            std::vector<VertexAtom> newVertexAtoms(numVertices);
            for (vNew = 0; vNew < numVertices; ++vNew)
            {
                newVertexAtoms[vNew] = mVertexAtoms[vertexNewToOld[vNew]];
            }
            //mVertexAtoms = newVertexAtoms;
            for (int32_t i = 0; i < numVertices; ++i)
            {
                mVertexAtoms[i] = newVertexAtoms[i];
            }

            // Construct the mappings between the old triangle order and the
            // new triangle order.
            std::vector<int32_t> triangleNewToOld(mNumTriangles);
            int32_t tNew = mNumTriangles - 1;

            for (auto const& collapse : mCollapses)
            {
                int32_t tOld = collapse.tThrow0;
                triangleNewToOld[tNew--] = tOld;
                tOld = collapse.tThrow1;
                triangleNewToOld[tNew--] = tOld;
            }

            for (auto tOld : mTrianglesRemaining)
            {
                triangleNewToOld[tNew--] = tOld;
            }

            // Reorder the index buffer.
            std::vector<int32_t> newIndices(mIndices.size());
            for (tNew = 0; tNew < mNumTriangles; ++tNew)
            {
                int32_t tOld = triangleNewToOld[tNew];
                for (size_t j = 0; j < 3; ++j)
                {
                    newIndices[3 * static_cast<size_t>(tNew) + j] =
                        mIndices[3 * static_cast<size_t>(tOld) + j];
                }
            }
            mIndices = newIndices;

            // Map the old indices to the new indices.
            for (size_t i = 0; i < mIndices.size(); ++i)
            {
                mIndices[i] = vertexOldToNew[mIndices[i]];
            }

            // Map the keep and throw vertices.
            for (auto& collapse : mCollapses)
            {
                collapse.vKeep = vertexOldToNew[collapse.vKeep];
                collapse.vThrow = vertexOldToNew[collapse.vThrow];
            }
        }

        void ComputeRecords(std::vector<CLODCollapseRecord>& records)
        {
            records.resize(mCollapses.size() + 1);

            // The initial record stores only the initial numbers of vertices
            // and triangles.
            int32_t numVertices = static_cast<int32_t>(mVertexAtoms.size());
            records[0].numVertices = numVertices;
            records[0].numTriangles = mNumTriangles;

            // Replace throw vertices in the index buffer as we process each
            // collapse record.
            std::vector<int32_t> indices = mIndices;
            std::vector<int32_t> vthrowIndices(mIndices.size());

            // Process the collapse records.
            CLODCollapseRecord* record = &records[1];
            int32_t numTriangles = mNumTriangles;
            for (auto const& collapse : mCollapses)
            {
                record->vKeep = collapse.vKeep;
                record->vThrow = collapse.vThrow;

                // An edge collapse loses one vertex.
                --numVertices;
                record->numVertices = numVertices;

                // An edge collapse loses two triangles.
                numTriangles -= 2;
                record->numTriangles = numTriangles;

                // Collapse the edge and update the indices for the
                // post-collapse index buffer.
                int32_t const numIndices = 3 * numTriangles;
                int32_t numRecordIndices = 0;
                for (int32_t i = 0; i < numIndices; ++i)
                {
                    if (indices[i] == record->vThrow)
                    {
                        vthrowIndices[numRecordIndices++] = i;
                        indices[i] = record->vKeep;
                    }
                }
                if (numRecordIndices > 0)
                {
                    record->indices.resize(numRecordIndices);
                    for (int32_t j = 0; j < numRecordIndices; ++j)
                    {
                        record->indices[j] = vthrowIndices[j];
                    }
                }
                else
                {
                    record->indices.clear();
                }

                ++record;
            }
        }

        // Triangle mesh to be decimated.
        std::vector<VertexAtom> mVertexAtoms;
        std::vector<int32_t> mIndices;

        // The vertex-edge-triangle graph.
        VertexArray mVertices;
        EdgeMap mEdges;
        TriangleMap mTriangles;
        int32_t mNumTriangles;

        // The edge heap to support collapse operations.
        MinHeap<EdgeKey<false>, float> mHeap;

        // The sequence of edge collapses.
        std::vector<CollapseInfo> mCollapses;

        // Postprocessing of the edge collapses.
        std::vector<int32_t> mVerticesRemaining;
        std::vector<Triangle> mTrianglesRemaining;
    };
}
