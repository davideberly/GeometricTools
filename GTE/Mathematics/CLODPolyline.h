// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#pragma once

// The continuous level of detail (CLOD) algorithm implemented here is
// described in
// https://www.geometrictools.com/Documentation/PolylineReduction.pdf
// It is an algorithm to reduce incrementally the number of vertices in a
// polyline (open or closed).  The sequence of vertex collapses is based on
// vertex weights associated with distance from vertices to polyline segments.

#include <Mathematics/Logger.h>
#include <Mathematics/MinHeap.h>
#include <Mathematics/DistPointSegment.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace gte
{
    template <int32_t N, typename Real>
    class CLODPolyline
    {
    public:
        // Constructor and destructor.  The number of vertices must be 2 or
        // larger.  The vertices are assumed to be ordered.  The segments are
        // <V[i],V[i+1]> for 0 <= i <= numVertices-2 for an open polyline.
        // If the polyline is closed, an additional segment is
        // <V[numVertices-1],V[0]>.
        CLODPolyline(std::vector<Vector<N, Real>> const& vertices, bool closed)
            :
            mNumVertices(static_cast<int32_t>(vertices.size())),
            mVertices(vertices),
            mClosed(closed),
            mNumEdges(0),
            mVMin(mClosed ? 3 : 2),
            mVMax(mNumVertices)
        {
            LogAssert(closed ? mNumVertices >= 3 : mNumVertices >= 2, "Invalid inputs.");

            // Compute the sequence of vertex collapses.  The polyline starts
            // out at the full level of detail (mNumVertices equals mVMax).
            VertexCollapse collapser;
            collapser(mVertices, mClosed, mIndices, mNumEdges, mEdges);
        }

        ~CLODPolyline() = default;

        // Member access.
        inline int32_t GetNumVertices() const
        {
            return mNumVertices;
        }

        inline std::vector<Vector<N, Real>> const& GetVertices() const
        {
            return mVertices;
        }

        inline bool GetClosed() const
        {
            return mClosed;
        }

        inline int32_t GetNumEdges() const
        {
            return mNumEdges;
        }

        inline std::vector<int32_t> const& GetEdges() const
        {
            return mEdges;
        }

        // Accessors to level of detail (MinLOD <= LOD <= MaxLOD is required).
        inline int32_t GetMinLevelOfDetail() const
        {
            return mVMin;
        }

        inline int32_t GetMaxLevelOfDetail() const
        {
            return mVMax;
        }

        inline int32_t GetLevelOfDetail() const
        {
            return mNumVertices;
        }

        void SetLevelOfDetail(int32_t numVertices)
        {
            if (numVertices < mVMin || numVertices > mVMax)
            {
                return;
            }

            // Decrease the level of detail.
            while (mNumVertices > numVertices)
            {
                --mNumVertices;
                mEdges[mIndices[mNumVertices]] = mEdges[2 * static_cast<size_t>(mNumEdges) - 1];
                --mNumEdges;
            }

            // Increase the level of detail.
            while (mNumVertices < numVertices)
            {
                ++mNumEdges;
                mEdges[mIndices[mNumVertices]] = mNumVertices;
                ++mNumVertices;
            }
        }

    private:
        // Support for vertex collapses in a polyline.
        class VertexCollapse
        {
        public:
            void operator()(std::vector<Vector<N, Real>>& vertices,
                bool closed, std::vector<int32_t>& indices, int32_t& numEdges,
                std::vector<int32_t>& edges)
            {
                int32_t numVertices = static_cast<int32_t>(vertices.size());
                indices.resize(vertices.size());

                if (closed)
                {
                    numEdges = numVertices;
                    edges.resize(2 * static_cast<size_t>(numEdges));

                    if (numVertices == 3)
                    {
                        indices[0] = 0;
                        indices[1] = 1;
                        indices[2] = 3;
                        edges[0] = 0;  edges[1] = 1;
                        edges[2] = 1;  edges[3] = 2;
                        edges[4] = 2;  edges[5] = 0;
                        return;
                    }
                }
                else
                {
                    numEdges = numVertices - 1;
                    edges.resize(2 * static_cast<size_t>(numEdges));

                    if (numVertices == 2)
                    {
                        indices[0] = 0;
                        indices[1] = 1;
                        edges[0] = 0;  edges[1] = 1;
                        return;
                    }
                }

                // Create the heap of weights.
                MinHeap<int32_t, Real> heap(numVertices);
                int32_t qm1 = numVertices - 1;
                if (closed)
                {
                    int32_t qm2 = numVertices - 2;
                    heap.Insert(0, GetWeight(qm1, 0, 1, vertices));
                    heap.Insert(qm1, GetWeight(qm2, qm1, 0, vertices));
                }
                else
                {
                    heap.Insert(0, std::numeric_limits<Real>::max());
                    heap.Insert(qm1, std::numeric_limits<Real>::max());
                }
                for (int32_t m = 0, z = 1, p = 2; z < qm1; ++m, ++z, ++p)
                {
                    heap.Insert(z, GetWeight(m, z, p, vertices));
                }

                // Create the level of detail information for the polyline.
                std::vector<int32_t> collapses(numVertices);
                CollapseVertices(heap, numVertices, collapses);
                ComputeEdges(numVertices, closed, collapses, indices, numEdges, edges);
                ReorderVertices(numVertices, vertices, collapses, numEdges, edges);
            }

        protected:
            // Weight calculation for vertex triple <V[m],V[z],V[p]>.
            Real GetWeight(int32_t m, int32_t z, int32_t p, std::vector<Vector<N, Real>>& vertices)
            {
                Vector<N, Real> direction = vertices[p] - vertices[m];
                Real length = Normalize(direction);
                if (length > (Real)0)
                {
                    Segment<N, Real> segment(vertices[m], vertices[p]);
                    DCPQuery<Real, Vector<N, Real>, Segment<N, Real>> query;
                    Real distance = query(vertices[z], segment).distance;
                    return distance / length;
                }
                else
                {
                    return std::numeric_limits<Real>::max();
                }
            }

            // Create data structures for the polyline.
            void CollapseVertices(MinHeap<int32_t, Real>& heap,
                int32_t numVertices, std::vector<int32_t>& collapses)
            {
                for (int32_t i = numVertices - 1; i >= 0; --i)
                {
                    Real weight;
                    heap.Remove(collapses[i], weight);
                }
            }

            void ComputeEdges(int32_t numVertices, bool closed, std::vector<int32_t>& collapses,
                std::vector<int32_t>& indices, int32_t numEdges, std::vector<int32_t>& edges)
            {
                // Compute the edges (first to collapse is last in array).  Do
                // not collapse the last line segment of an open polyline.  Do
                // not collapse further when a closed polyline becomes a
                // triangle.
                int32_t i, vIndex, eIndex = 2 * numEdges - 1;
                if (closed)
                {
                    for (i = numVertices - 1; i >= 0; --i)
                    {
                        vIndex = collapses[i];
                        edges[eIndex--] = (vIndex + 1) % numVertices;
                        edges[eIndex--] = vIndex;
                    }
                }
                else
                {
                    for (i = numVertices - 1; i >= 2; --i)
                    {
                        vIndex = collapses[i];
                        edges[eIndex--] = vIndex + 1;
                        edges[eIndex--] = vIndex;
                    }

                    vIndex = collapses[0];
                    edges[0] = vIndex;
                    edges[1] = vIndex + 1;
                }

                // In the given edge order, find the index in the edge array
                // that corresponds to a collapse vertex and save the index
                // for the dynamic change in level of detail.  This relies on
                // the assumption that a vertex is shared by at most two
                // edges.
                eIndex = 2 * numEdges - 1;
                for (i = numVertices - 1; i >= 0; --i)
                {
                    vIndex = collapses[i];
                    for (int32_t e = 0; e < 2 * numEdges; ++e)
                    {
                        if (vIndex == edges[e])
                        {
                            indices[i] = e;
                            edges[e] = edges[eIndex];
                            break;
                        }
                    }
                    eIndex -= 2;

                    if (closed)
                    {
                        if (eIndex == 5)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if (eIndex == 1)
                        {
                            break;
                        }
                    }
                }

                // Restore the edge array to full level of detail.
                if (closed)
                {
                    for (i = 3; i < numVertices; ++i)
                    {
                        edges[indices[i]] = collapses[i];
                    }
                }
                else
                {
                    for (i = 2; i < numVertices; ++i)
                    {
                        edges[indices[i]] = collapses[i];
                    }
                }
            }

            void ReorderVertices(int32_t numVertices, std::vector<Vector<N, Real>>& vertices,
                std::vector<int32_t>& collapses, int32_t numEdges, std::vector<int32_t>& edges)
            {
                std::vector<int32_t> permute(numVertices);
                std::vector<Vector<N, Real>> permutedVertex(numVertices);

                for (int32_t i = 0; i < numVertices; ++i)
                {
                    int32_t vIndex = collapses[i];
                    permute[vIndex] = i;
                    permutedVertex[i] = vertices[vIndex];
                }

                for (int32_t i = 0; i < 2 * numEdges; ++i)
                {
                    edges[i] = permute[edges[i]];
                }

                vertices = permutedVertex;
            }
        };

    private:
        // The polyline vertices.
        int32_t mNumVertices;
        std::vector<Vector<N, Real>> mVertices;
        bool mClosed;

        // The polyline edges.
        int32_t mNumEdges;
        std::vector<int32_t> mEdges;

        // The level of detail information.
        int32_t mVMin, mVMax;
        std::vector<int32_t> mIndices;
    };
}
