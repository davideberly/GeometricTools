// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.03

#pragma once

#include <Mathematics/Delaunay2.h>
#include <list>

// Various parts of the code have LogAssert or LogError tests.  For a correct
// algorithm using exact arithmetic, we do not expect to trigger these.
// However, with floating-point arithmetic, it is possible that the
// triangulation becomes malformed.  The calls to the member function
// Insert(...) should be made in a try-catch block.  If an exception is
// thrown, you are most likely using a floating-point type for ComputeType
// and floating-point rounding errors have caused problems in the edge
// insertions.

namespace gte
{
    template <typename InputType, typename ComputeType>
    class ConstrainedDelaunay2 : public Delaunay2<InputType, ComputeType>
    {
    public:
        // The class is a functor to support computing the constrained
        // Delaunay triangulation of multiple data sets using the same class
        // object.
        virtual ~ConstrainedDelaunay2() = default;

        ConstrainedDelaunay2()
            :
            Delaunay2<InputType, ComputeType>()
        {
        }

        // This operator computes the Delaunay triangulation only.  Read the
        // Delaunay2 constructor comments about 'vertices' and 'epsilon'.  The
        // 'edges' array has indices into the 'vertices' array.  No two edges
        // should intersect except at endpoints.
        bool operator()(int numVertices, Vector2<InputType> const* vertices, InputType epsilon)
        {
            return Delaunay2<InputType, ComputeType>::operator()(numVertices, vertices, epsilon);
        }

        // Insert required edges into the triangulation.  For correctness of
        // the algorithm, if two edges passed to this function intersect, they
        // must do so only at vertices passed to operator().  If you have two
        // edges that intersect at a point not in the vertices, compute that
        // point of intersection and subdivide the edges at that intersection
        // (to form more edges), and add the point to the vertices before
        // calling operator().  This function has an output array that
        // contains the input edge when the only vertices on the edge are its
        // endpoints.  If the input edge passes through more vertices, the
        // edge is subdivided in this function.  The output edge is that
        // subdivision with first vertex edge[0] and last vertex edge[1], and
        // the other vertices are correctly ordered along the edge.
        bool Insert(std::array<int, 2> const& edge, std::vector<int>& outEdge)
        {
            int v0 = edge[0], v1 = edge[1];
            if (0 <= v0 && v0 < this->mNumVertices
                && 0 <= v1 && v1 < this->mNumVertices)
            {
                int v0Triangle = GetLinkTriangle(v0);
                if (v0Triangle >= 0)
                {
                    // Once an edge is inserted, the base-class mGraph no
                    // longer represents the triangulation.  Clear it in case
                    // the user tries to access it.
                    this->mGraph.Clear();

                    outEdge.clear();
                    return Insert(edge, v0Triangle, outEdge);
                }
            }
            return false;
        }

    private:
        // The top-level entry point for inserting an edge in the
        // triangulation.
        bool Insert(std::array<int, 2> const& edge, int v0Triangle, std::vector<int>& outEdge)
        {
            // Create the neighborhood of triangles that share the vertex v0.
            // On entry we already know one such triangle (v0Triangle).
            int v0 = edge[0], v1 = edge[1];
            std::list<std::pair<int, int>> link;
            bool isOpen = true;
            bool success = BuildLink(v0, v0Triangle, link, isOpen);
            LogAssert(success, CDTFailure());

            // Determine which triangle contains the edge.  Process the edge
            // according to whether it is strictly between two triangle edges
            // or is coincident with a triangle edge.
            auto item = link.begin();
            std::array<int, 3> indices;
            success = this->GetIndices(item->first, indices);
            LogAssert(success, CDTFailure());

            int vNext = indices[(item->second + 1) % 3];
            int qr0 = this->mQuery.ToLine(v1, v0, vNext);
            while (item != link.end())
            {
                if (qr0 == 0)
                {
                    // We have to be careful about parallel edges that point
                    // in the opposite direction of <v0,v1>.
                    Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
                    Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
                    Vector2<ComputeType> const& ctvnext = this->mComputeVertices[vNext];
                    if (Dot(ctv1 - ctv0, ctvnext - ctv0) > (ComputeType)0)
                    {
                        // <v0,v1> is coincident to triangle edge0.
                        return ProcessCoincident(item->first, v0, v1, vNext, outEdge);
                    }

                    // Make sure we enter the next "if" statement to continue
                    // traversing the link.
                    qr0 = 1;
                }

                if (qr0 > 0)
                {
                    // <v0,v1> is not in triangle.  Visit the next triangle.
                    if (++item == link.end())
                    {
                        return false;
                    }
                    success = this->GetIndices(item->first, indices);
                    LogAssert(success, CDTFailure());
                    vNext = indices[(item->second + 1) % 3];
                    qr0 = this->mQuery.ToLine(v1, v0, vNext);
                    continue;
                }

                int vPrev = indices[(item->second + 2) % 3];
                int qr1 = this->mQuery.ToLine(v1, v0, vPrev);
                while (item != link.end())
                {
                    if (qr1 == 0)
                    {
                        // We have to be careful about parallel edges that
                        // point in the opposite direction of <v0,v1>.
                        Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
                        Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
                        Vector2<ComputeType> const& ctvprev =
                            this->mComputeVertices[vPrev];
                        if (Dot(ctv1 - ctv0, ctvprev - ctv0) > (ComputeType)0)
                        {
                            // <v0,v1> is coincident to triangle edge1.
                            return ProcessCoincident(item->first, v0, v1, vPrev, outEdge);
                        }

                        // Make sure we enter the next "if" statement to
                        // continue traversing the link.
                        qr1 = -1;
                    }

                    if (qr1 < 0)
                    {
                        // <v0,v1> is not in triangle.  Visit the next
                        // triangle.
                        if (++item == link.end())
                        {
                            return false;
                        }
                        this->GetIndices(item->first, indices);
                        vNext = vPrev;
                        vPrev = indices[(item->second + 2) % 3];
                        qr1 = this->mQuery.ToLine(v1, v0, vPrev);
                        continue;
                    }

                    // <v0,v1> is interior to triangle <v0,vNext,vPrev>.
                    return ProcessInterior(item->first, v0, v1, vNext, vPrev, outEdge);
                }
                break;
            }

            // The edge must be contained in some link triangle.
            LogError(CDTFailure());
        }

        // Process the coincident edge.
        bool ProcessCoincident(int tri, int v0, int v1, int vOther, std::vector<int>& outEdge)
        {
            outEdge.push_back(v0);
            if (v1 != vOther)
            {
                // Decompose the edge and process the right-most subedge.
                return Insert({ vOther, v1 }, tri, outEdge);
            }
            else
            {
                // <v0,v1> is already in the triangulation.
                outEdge.push_back(v1);
                return true;
            }
        }

        // Process the triangle strip originating at the first endpoint of the
        // edge.
        bool ProcessInterior(int tri, int v0, int v1, int vNext, int vPrev, std::vector<int>& outEdge)
        {
            // The triangles of the strip are stored in 'polygon'.  The
            // retriangulation leads to the same number of triangles, so we
            // can reuse the mIndices[] and mAdjacencies[] locations implied
            // by the 'polygons' indices.
            std::vector<int> polygon;

            // The sBoundary[i] (s in {l,r}) array element is <v0,adj>; see
            // the header comments for GetAdjBoundary about what these mean.
            // The boundary vertex is 'v0', the adjacent triangle 'adj' is
            // outside the strip and shares the edge <sBoundary[i-1][0],
            // sBoundary[i][0]> with a triangle in 'polygon'.  This
            // information allows us to connect the adjacent triangles outside
            // the strip to new triangles inserted by the retriangulation.
            // The value sBoundary[0][1,2] values are set to -1 but they are
            // not used in the construction.
            std::vector<std::array<int, 2>> lBoundary, rBoundary;
            std::array<int, 2> binfo;

            polygon.push_back(tri);

            lBoundary.push_back({ v0, -1 });
            binfo = GetAdjBoundary(tri, vPrev, vPrev);
            lBoundary.push_back(binfo);

            rBoundary.push_back({ v0, -1 });
            binfo = GetAdjBoundary(tri, vNext, v0);
            rBoundary.push_back(binfo);

            // Visit the triangles in the strip.  Guard against an infinite
            // loop.
            for (int i = 0; i < this->mNumTriangles; ++i)
            {
                // Find the vertex of the adjacent triangle that is opposite
                // the edge <vNext,vPrev> shared with the current triangle.
                auto iinfo = GetAdjInterior(tri, vNext, vPrev);
                int adj = iinfo[0], vOpposite = iinfo[1];
                LogAssert(vOpposite >= 0, CDTFailure());

                // Visit the adjacent triangle and insert it into the polygon.
                tri = adj;
                polygon.push_back(tri);

                int qr = this->mQuery.ToLine(vOpposite, v0, v1);
                if (qr == 0)
                {
                    // We have encountered a vertex that terminates the
                    // triangle strip.  Retriangulate the polygon.  If the
                    // edge continues through vOpposite, decompose the edge
                    // and insert the right-most subedge.
                    binfo = GetAdjBoundary(tri, vOpposite, vOpposite);
                    lBoundary.push_back(binfo);

                    binfo = GetAdjBoundary(tri, vOpposite, vNext);
                    rBoundary.push_back(binfo);

                    Retriangulate(polygon, lBoundary, rBoundary);
                    if (vOpposite != v1)
                    {
                        outEdge.push_back(v0);
                        return Insert({ vOpposite, v1 }, tri, outEdge);
                    }
                    else
                    {
                        return true;
                    }
                }

                if (qr < 0)
                {
                    binfo = GetAdjBoundary(tri, vOpposite, vOpposite);
                    lBoundary.push_back(binfo);
                    vPrev = vOpposite;
                }
                else  // qr > 0
                {
                    binfo = GetAdjBoundary(tri, vOpposite, vNext);
                    rBoundary.push_back(binfo);
                    vNext = vOpposite;
                }
            }

            // The triangle strip should have been located in the loop.
            LogError(CDTFailure());
        }

        // Remove the triangles in the triangle strip and retriangulate the
        // left and right polygons using the empty circumcircle condition.
        bool Retriangulate(std::vector<int>& polygon,
            std::vector<std::array<int, 2>> const& lBoundary,
            std::vector<std::array<int, 2>> const& rBoundary)
        {
            int t0 = RetriangulateLRecurse(lBoundary, 0,
                static_cast<int>(lBoundary.size()) - 1, -1, polygon);

            int t1 = RetriangulateRRecurse(rBoundary, 0,
                static_cast<int>(rBoundary.size()) - 1, -1, polygon);

            int v0 = lBoundary.front()[0];
            int v1 = lBoundary.back()[0];
            bool success = Connect(t0, t1, v0, v1);
            LogAssert(success, CDTFailure());
            return true;
        }

        int RetriangulateLRecurse(
            std::vector<std::array<int, 2>> const& lBoundary,
            int i0, int i1, int a0, std::vector<int>& polygon)
        {
            // Create triangles when recursing down, connect adjacent
            // triangles when returning.

            int v0 = lBoundary[i0][0];
            int v1 = lBoundary[i1][0];

            bool success;
            if (i1 - i0 == 1)
            {
                success = Connect(a0, lBoundary[i1][1], v1, v0);
                LogAssert(success, CDTFailure());
                return -1;  // No triangle created.
            }
            else
            {
                // Select i2 in [i0+1,i1-1] for minimum distance to edge
                // <i0,i1>.
                int i2 = SelectSplit(lBoundary, i0, i1);
                int v2 = lBoundary[i2][0];

                // Reuse a triangle and fill in its new vertices.
                int tri = polygon.back();
                polygon.pop_back();
                this->mIndices[3 * tri + 0] = v0;
                this->mIndices[3 * tri + 1] = v1;
                this->mIndices[3 * tri + 2] = v2;

                // Recurse downward and create triangles.
                int ret0 = RetriangulateLRecurse(lBoundary, i0, i2, tri, polygon);
                LogAssert(ret0 >= -1, CDTFailure());
                int ret1 = RetriangulateLRecurse(lBoundary, i2, i1, tri, polygon);
                LogAssert(ret1 >= -1, CDTFailure());

                // Return and connect triangles.
                success = Connect(a0, tri, v1, v0);
                LogAssert(success, CDTFailure())
                return tri;
            }
        }

        int RetriangulateRRecurse(
            std::vector<std::array<int, 2>> const& rBoundary,
            int i0, int i1, int a0, std::vector<int>& polygon)
        {
            // Create triangles when recursing down, connect adjacent
            // triangles when returning.

            int v0 = rBoundary[i0][0];
            int v1 = rBoundary[i1][0];

            if (i1 - i0 == 1)
            {
                bool success = Connect(a0, rBoundary[i1][1], v0, v1);
                LogAssert(success, CDTFailure());
                return -1;  // No triangle created.
            }
            else
            {
                // Select i2 in [i0+1,i1-1] for minimum distance to edge
                // <i0,i1>.
                int i2 = SelectSplit(rBoundary, i0, i1);
                int v2 = rBoundary[i2][0];

                // Reuse a triangle and fill in its new vertices.
                int tri = polygon.back();
                polygon.pop_back();
                this->mIndices[3 * tri + 0] = v1;
                this->mIndices[3 * tri + 1] = v0;
                this->mIndices[3 * tri + 2] = v2;

                // Recurse downward and create triangles.
                int ret0 = RetriangulateRRecurse(rBoundary, i0, i2, tri, polygon);
                LogAssert(ret0 >= -1, CDTFailure());
                int ret1 = RetriangulateRRecurse(rBoundary, i2, i1, tri, polygon);
                LogAssert(ret1 >= -1, CDTFailure());

                // Return and connect triangles.
                bool success = Connect(a0, tri, v0, v1);
                LogAssert(success, CDTFailure());
                return tri;
            }
        }

        int SelectSplit(std::vector<std::array<int, 2>> const& boundary, int i0, int i1) const
        {
            int i2;
            if (i1 - i0 == 2)
            {
                // This is the only candidate.
                i2 = i0 + 1;
            }
            else  // i1 - i0 > 2
            {
                // Select the index i2 in [i0+1,i1-1] for which the distance
                // from the vertex v2 at i2 to the edge <v0,v1> is minimized.
                // To allow exact arithmetic, use a pseudosquared distance
                // that avoids divisions and square roots.
                i2 = i0 + 1;
                int v0 = boundary[i0][0];
                int v1 = boundary[i1][0];
                int v2 = boundary[i2][0];
                ComputeType minpsd = ComputePSD(v0, v1, v2);
                for (int i = i2 + 1; i < i1; ++i)
                {
                    v2 = boundary[i][0];
                    ComputeType psd = ComputePSD(v0, v1, v2);
                    if (psd < minpsd)
                    {
                        minpsd = psd;
                        i2 = i;
                    }
                }
            }
            return i2;
        }

        // Compute a pseudosquared distance from the vertex at v2 to the edge
        // <v0,v1>.
        ComputeType ComputePSD(int v0, int v1, int v2) const
        {
            ComputeType psd;

            Vector2<ComputeType> const& ctv0 = this->mComputeVertices[v0];
            Vector2<ComputeType> const& ctv1 = this->mComputeVertices[v1];
            Vector2<ComputeType> const& ctv2 = this->mComputeVertices[v2];

            Vector2<ComputeType> V1mV0 = ctv1 - ctv0;
            Vector2<ComputeType> V2mV0 = ctv2 - ctv0;
            ComputeType sqrlen10 = Dot(V1mV0, V1mV0);
            ComputeType dot = Dot(V1mV0, V2mV0);
            ComputeType zero(0);

            if (dot <= zero)
            {
                ComputeType sqrlen20 = Dot(V2mV0, V2mV0);
                psd = sqrlen10 * sqrlen20;
            }
            else
            {
                Vector2<ComputeType> V2mV1 = ctv2 - ctv1;
                dot = Dot(V1mV0, V2mV1);
                if (dot >= zero)
                {
                    ComputeType sqrlen21 = Dot(V2mV1, V2mV1);
                    psd = sqrlen10 * sqrlen21;
                }
                else
                {
                    dot = DotPerp(V2mV0, V1mV0);
                    psd = sqrlen10 * dot * dot;
                }
            }

            return psd;
        }

        // Search the triangulation for a triangle that contains the specified
        // vertex.
        int GetLinkTriangle(int v) const
        {
            // Remap in case an edge vertex was specified that is a duplicate.
            v = this->mDuplicates[v];

            int tri = 0;
            for (int i = 0; i < this->mNumTriangles; ++i)
            {
                // Test whether v is a vertex of the triangle.
                std::array<int, 3> indices;
                bool success = this->GetIndices(tri, indices);
                LogAssert(success, CDTFailure());
                for (int j = 0; j < 3; ++j)
                {
                    if (v == indices[j])
                    {
                        return tri;
                    }
                }

                // v must be outside the triangle.
                for (int j0 = 2, j1 = 0; j1 < 3; j0 = j1++)
                {
                    if (this->mQuery.ToLine(v, indices[j0], indices[j1]) > 0)
                    {
                        // Vertex v sees the edge from outside, so traverse to
                        // the triangle sharing the edge.
                        std::array<int, 3> adjacencies;
                        success = this->GetAdjacencies(tri, adjacencies);
                        LogAssert(success, CDTFailure());
                        int adj = adjacencies[j0];
                        LogAssert(adj >= 0, CDTFailure());
                        tri = adj;
                        break;
                    }
                }
            }

            // The vertex must be in the triangulation.
            LogError(CDTFailure());
        }

        // Determine the index in {0,1,2} for the triangle 'tri' that contains
        // the vertex 'v'.
        int GetIndexOfVertex(int tri, int v) const
        {
            std::array<int, 3> indices;
            bool success = this->GetIndices(tri, indices);
            LogAssert(success, CDTFailure());
            int indexOfV;
            for (indexOfV = 0; indexOfV < 3; ++indexOfV)
            {
                if (v == indices[indexOfV])
                {
                    return indexOfV;
                }
            }

            LogError(CDTFailure());
        }

        // Given a triangle 'tri' with CCW-edge <v0,v1>, return <adj,v2> where
        // 'adj' is the index of the triangle adjacent to 'tri' that shares
        // the edge and 'v2' is the vertex of the adjacent triangle opposite
        // the edge.  This function supports traversing a triangle strip that
        // contains a constraint edge, so it is called only when an adjacent
        // triangle actually exists.
        std::array<int, 2> GetAdjInterior(int tri, int v0, int v1) const
        {
            int vIndex = GetIndexOfVertex(tri, v0);
            LogAssert(vIndex >= 0, CDTFailure());
            int adj = this->mAdjacencies[3 * tri + vIndex];
            if (adj >= 0)
            {
                for (int v2Index = 0; v2Index < 3; ++v2Index)
                {
                    int v2 = this->mIndices[3 * adj + v2Index];
                    if (v2 != v0 && v2 != v1)
                    {
                        return{ adj, v2 };
                    }
                }
            }
            else
            {
                return{ -1, -1 };
            }

            LogError(CDTFailure());
        }

        // Given a triangle 'tri' of the triangle strip, the boundary edge
        // must contain the vertex with index 'needBndVertex'.  The input
        // 'needAdjVIndex' specifies where to look for the index of the
        // triangle outside the strip but adjacent to the boundary edge.  The
        // return value is <needBndVertex, adj> and is used to connect 'tri'
        // and 'adj' across a triangle strip boundary.
        std::array<int, 2> GetAdjBoundary(int tri, int needBndVertex, int needAdjVIndex) const
        {
            int vIndex = GetIndexOfVertex(tri, needAdjVIndex);
            LogAssert(vIndex >= 0, CDTFailure());
            int adj = this->mAdjacencies[3 * tri + vIndex];
            return{ needBndVertex, adj };
        }

        // Set the indices and adjacencies arrays so that 'tri' and 'adj'
        // share the common edge; 'tri' has CCW-edge <v0,v1> and 'adj' has
        // CCW-edge <v1,v0>.
        bool Connect(int tri, int adj, int v0, int v1)
        {
            if (tri >= 0)
            {
                int v0Index = GetIndexOfVertex(tri, v0);
                LogAssert(v0Index >= 0, CDTFailure());
                if (adj >= 0)
                {
                    int v1Index = GetIndexOfVertex(adj, v1);
                    LogAssert(v1Index >= 0, CDTFailure());
                    this->mAdjacencies[3 * adj + v1Index] = tri;
                }

                this->mAdjacencies[3 * tri + v0Index] = adj;
            }
            // else: tri = -1, which occurs in the top-level call to
            // retriangulate
            return true;
        }

        // Create an ordered list of triangles forming the link of a vertex.
        // The pair of the list is <triangle,GetIndexOfV(triangle)>.  This
        // allows us to cache the index of v relative to each triangle in the
        // link.  The vertex v might be a boundary vertex, in which case the
        // neighborhood is open; otherwise, v is an interior vertex and the
        // neighborhood is closed.  The 'isOpen' parameter specifies the case.
        bool BuildLink(int v, int vTriangle, std::list<std::pair<int, int>>& link, bool& isOpen) const
        {
            // The list starts with a known triangle in the link of v.
            int vStartIndex = GetIndexOfVertex(vTriangle, v);
            LogAssert(vStartIndex >= 0, CDTFailure());
            link.push_front(std::make_pair(vTriangle, vStartIndex));

            // Traverse adjacent triangles to the "left" of v.  Guard against
            // an infinite loop.
            int tri = vTriangle, vIndex = vStartIndex;
            std::array<int, 3> adjacencies;
            for (int i = 0; i < this->mNumTriangles; ++i)
            {
                bool success = this->GetAdjacencies(tri, adjacencies);
                LogAssert(success, CDTFailure());
                int adjPrev = adjacencies[(vIndex + 2) % 3];
                if (adjPrev >= 0)
                {
                    if (adjPrev != vTriangle)
                    {
                        tri = adjPrev;
                        vIndex = GetIndexOfVertex(tri, v);
                        LogAssert(vIndex >= 0, CDTFailure());
                        link.push_back(std::make_pair(tri, vIndex));
                    }
                    else
                    {
                        // We have reached the starting triangle, so v is an
                        // interior vertex.
                        isOpen = false;
                        return true;
                    }
                }
                else
                {
                    // We have reached a triangle with boundary edge, so v is
                    // a boundary vertex.  We mush find more triangles by
                    // searching to the "right" of v.  Guard against an
                    // infinite loop.
                    isOpen = true;
                    tri = vTriangle;
                    vIndex = vStartIndex;
                    for (int j = 0; j < this->mNumTriangles; ++j)
                    {
                        this->GetAdjacencies(tri, adjacencies);
                        int adjNext = adjacencies[vIndex];
                        if (adjNext >= 0)
                        {
                            tri = adjNext;
                            vIndex = GetIndexOfVertex(tri, v);
                            LogAssert(vIndex >= 0, CDTFailure());
                            link.push_front(std::make_pair(tri, vIndex));
                        }
                        else
                        {
                            // We have reached the other boundary edge that
                            // shares v.
                            return true;
                        }
                    }
                    break;
                }
            }

            LogError(CDTFailure());
        }

        static std::string const& CDTFailure()
        {
            static std::string message = "Unexpected condition. Caused by floating-point rounding error?";
            return message;
        }
    };
}
