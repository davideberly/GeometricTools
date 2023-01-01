// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

// Manifold Triangle Mesh.  Each edge has 1 or 2 triangles sharing it.

#include <Mathematics/Logger.h>
#include <Mathematics/EdgeKey.h>
#include <Mathematics/TriangleKey.h>
#include "MTVertex.h"
#include "MTEdge.h"
#include "MTTriangle.h"
#include <fstream>
#include <map>
#include <string>

class MTMesh
{
public:
    MTMesh(int32_t numVertices = 0, int32_t numEdges = 0, int32_t numTriangles = 0)
        :
        mVertices(numVertices),
        mEdges(numEdges),
        mTriangles(numTriangles),
        mInitialELabel(-1),
        mInitialTLabel(-1)
    {
    }

    virtual ~MTMesh() = default;

    void Reset(int32_t numVertices = 0, int32_t numEdges = 0, int32_t numTriangles = 0)
    {
        mVertices.Reset(numVertices);
        mEdges.Reset(numEdges);
        mTriangles.Reset(numTriangles);
        mInitialELabel = -1;
        mInitialTLabel = 1;
        mVMap.clear();
        mEMap.clear();
        mTMap.clear();
    }

    inline int32_t GetNumVertices() const
    {
        return mVertices.GetNumElements();
    }

    inline int32_t V(int32_t label) const
    {
        auto iter = mVMap.find(label);
        return (iter != mVMap.end() ? iter->second : -1);
    }

    inline MTVertex const& GetVertex(int32_t vIndex) const
    {
        return mVertices[vIndex];
    }

    inline int32_t GetVLabel(int32_t vIndex) const
    {
        return mVertices[vIndex].GetLabel();
    }

    inline int32_t GetNumEdges() const
    {
        return mEdges.GetNumElements();
    }

    inline int32_t E(int32_t label0, int32_t label1) const
    {
        auto iter = mEMap.find(gte::EdgeKey<false>(label0, label1));
        return (iter != mEMap.end() ? iter->second : -1);
    }

    inline MTEdge const& GetEdge(int32_t eIndex) const
    {
        return mEdges[eIndex];
    }

    inline int32_t GetELabel(int32_t eIndex) const
    {
        return mEdges[eIndex].GetLabel();
    }

    inline void SetELabel(int32_t eIndex, int32_t label)
    {
        mEdges[eIndex].SetLabel(label);
    }

    inline int32_t GetNumTriangles() const
    {
        return mTriangles.GetNumElements();
    }

    inline int32_t T(int32_t label0, int32_t label1, int32_t label2) const
    {
        auto iter = mTMap.find(gte::TriangleKey<false>(label0, label1, label2));
        return (iter != mTMap.end() ? iter->second : -1);
    }

    inline MTTriangle const& GetTriangle(int32_t tIndex) const
    {
        return mTriangles[tIndex];
    }

    inline int32_t GetTLabel(int32_t tIndex) const
    {
        return mTriangles[tIndex].GetLabel();
    }

    inline void SetTLabel(int32_t tIndex, int32_t label)
    {
        mTriangles[tIndex].SetLabel(label);
    }

    inline int32_t GetInitialELabel() const
    {
        return mInitialELabel;
    }

    inline void SetInitialELabel(int32_t label)
    {
        mInitialELabel = label;
    }

    inline int32_t GetInitialTLabel() const
    {
        return mInitialTLabel;
    }

    inline void SetInitialTLabel(int32_t label)
    {
        mInitialTLabel = label;
    }

    bool Insert(int32_t label0, int32_t label1, int32_t label2)
    {
        // Insert the triangle.
        int32_t t = InsertTriangle(label0, label1, label2);
        if (t == -1)
        {
            // The triangle already exists.
            return true;
        }

        // Insert the vertices of the triangle.
        int32_t v0 = InsertVertex(label0);
        int32_t v1 = InsertVertex(label1);
        int32_t v2 = InsertVertex(label2);

        // Insert the edges of the triangle.
        int32_t e0 = InsertEdge(label0, label1);
        int32_t e1 = InsertEdge(label1, label2);
        int32_t e2 = InsertEdge(label2, label0);

        // Set the connections among the components.
        MTTriangle& triangle = mTriangles[t];
        MTVertex& vertex0 = mVertices[v0];
        MTVertex& vertex1 = mVertices[v1];
        MTVertex& vertex2 = mVertices[v2];
        MTEdge& edge0 = mEdges[e0];
        MTEdge& edge1 = mEdges[e1];
        MTEdge& edge2 = mEdges[e2];

        // Attach edges to vertices.
        vertex0.InsertEdge(e2);
        vertex0.InsertEdge(e0);
        vertex1.InsertEdge(e0);
        vertex1.InsertEdge(e1);
        vertex2.InsertEdge(e1);
        vertex2.InsertEdge(e2);
        edge0.SetVertex(0, v0);
        edge0.SetVertex(1, v1);
        edge1.SetVertex(0, v1);
        edge1.SetVertex(1, v2);
        edge2.SetVertex(0, v2);
        edge2.SetVertex(1, v0);

        // Attach triangles to vertices.
        vertex0.InsertTriangle(t);
        vertex1.InsertTriangle(t);
        vertex2.InsertTriangle(t);
        triangle.SetVertex(0, v0);
        triangle.SetVertex(1, v1);
        triangle.SetVertex(2, v2);

        // Attach triangle to edges.
        AttachTriangleToEdge(t, triangle, 0, e0, edge0);
        AttachTriangleToEdge(t, triangle, 1, e1, edge1);
        AttachTriangleToEdge(t, triangle, 2, e2, edge2);
        return true;
    }

    bool Remove(int32_t label0, int32_t label1, int32_t label2)
    {
        auto iter = mTMap.find(gte::TriangleKey<false>(label0, label1, label2));
        if (iter == mTMap.end())
        {
            // The triangle does not exist.
            return false;
        }
        int32_t t = iter->second;

        MTTriangle& triangle = mTriangles[t];

        // Detach triangle from edges.
        int32_t e0 = triangle.GetEdge(0);
        int32_t e1 = triangle.GetEdge(1);
        int32_t e2 = triangle.GetEdge(2);
        MTEdge& edge0 = mEdges[e0];
        MTEdge& edge1 = mEdges[e1];
        MTEdge& edge2 = mEdges[e2];
        DetachTriangleFromEdge(t, triangle, 0, e0, edge0);
        DetachTriangleFromEdge(t, triangle, 1, e1, edge1);
        DetachTriangleFromEdge(t, triangle, 2, e2, edge2);

        // Detach triangle from vertices.
        int32_t v0 = triangle.GetVertex(0);
        MTVertex& vertex0 = mVertices[v0];
        vertex0.RemoveTriangle(t);

        int32_t v1 = triangle.GetVertex(1);
        MTVertex& vertex1 = mVertices[v1];
        vertex1.RemoveTriangle(t);

        int32_t v2 = triangle.GetVertex(2);
        MTVertex& vertex2 = mVertices[v2];
        vertex2.RemoveTriangle(t);

        // Detach edges from vertices (only if last edge to reference vertex).
        bool e0Destroy = (edge0.GetTriangle(0) == -1);
        if (e0Destroy)
        {
            vertex0.RemoveEdge(e0);
            vertex1.RemoveEdge(e0);
        }

        bool e1Destroy = (edge1.GetTriangle(0) == -1);
        if (e1Destroy)
        {
            vertex1.RemoveEdge(e1);
            vertex2.RemoveEdge(e1);
        }

        bool e2Destroy = (edge2.GetTriangle(0) == -1);
        if (e2Destroy)
        {
            vertex0.RemoveEdge(e2);
            vertex2.RemoveEdge(e2);
        }

        // Removal of components from the sets and maps starts here.  Be
        // careful using set indices, component references, and map iterators
        // because deletion has side effects.  Deletion of a component might
        // cause another component to be moved within the corresponding set
        // or map.
        bool v0Destroy = (vertex0.GetNumEdges() == 0);
        bool v1Destroy = (vertex1.GetNumEdges() == 0);
        bool v2Destroy = (vertex2.GetNumEdges() == 0);

        // Remove edges if no longer used.
        if (e0Destroy)
        {
            RemoveEdge(label0, label1);
        }

        if (e1Destroy)
        {
            RemoveEdge(label1, label2);
        }

        if (e2Destroy)
        {
            RemoveEdge(label2, label0);
        }

        // Remove vertices if no longer used.
        if (v0Destroy)
        {
            RemoveVertex(label0);
        }

        if (v1Destroy)
        {
            RemoveVertex(label1);
        }

        if (v2Destroy)
        {
            RemoveVertex(label2);
        }

        // Remove triangle (definitely no longer used).
        RemoveTriangle(label0, label1, label2);
        return true;
    }

    bool SubdivideCentroid(int32_t label0, int32_t label1, int32_t label2, int32_t& nextLabel)
    {
        int32_t t = T(label0, label1, label2);
        if (t == -1)
        {
            return false;
        }

        if (mVMap.find(nextLabel) != mVMap.end())
        {
            // Vertex already exists with this label.
            return false;
        }

        // Subdivide the triangle.
        Remove(label0, label1, label2);
        Insert(label0, label1, nextLabel);
        Insert(label1, label2, nextLabel);
        Insert(label2, label0, nextLabel);

        ++nextLabel;
        return true;
    }

    bool SubdivideCentroidAll(int32_t& nextLabel)
    {
        // Verify that the next-label range is valid.
        int32_t const tMax = mTriangles.GetNumElements();
        int32_t t, tempLabel;
        for (t = 0, tempLabel = nextLabel; t < tMax; ++t, ++tempLabel)
        {
            if (mVMap.find(tempLabel) != mVMap.end())
            {
                // A vertex already exists with this label.
                return false;
            }
        }

        // Care must be taken when processing the triangles iteratively.  The
        // side of effect of removing the first triangle is that the last
        // triangle in the array is moved into the vacated position.  The
        // first problem is that the moved triangle will be skipped in the
        // iteration.  The second problem is that the insertions cause the
        // triangle array to grow.  To avoid skipping the moved triangle, a
        // different algorithm than the one in SubdivideCentroid(...) is used.
        // The triangle to be removed is detached from two edges.  Two of the
        // subtriangles are added to the mesh.  The third subtriangle is
        // calculated in the already existing memory that stored the original
        // triangle.  To avoid the infinite recursion induced by a growing
        // array, the original size of the triangle array is stored int32_t tMax.
        // This guarantees that only the original triangles are subdivided and
        // that newly added triangles are not.
        for (t = 0; t < tMax; ++t, ++nextLabel)
        {
            // The triangle to subdivide.
            MTTriangle& triangle = mTriangles[t];
            int32_t label0 = GetVLabel(triangle.GetVertex(0));
            int32_t label1 = GetVLabel(triangle.GetVertex(1));
            int32_t label2 = GetVLabel(triangle.GetVertex(2));

            // Detach the triangle from two edges.
            int32_t e1 = triangle.GetEdge(1);
            int32_t e2 = triangle.GetEdge(2);
            MTEdge& edge1 = mEdges[e1];
            MTEdge& edge2 = mEdges[e2];
            DetachTriangleFromEdge(t, triangle, 1, e1, edge1);
            DetachTriangleFromEdge(t, triangle, 2, e2, edge2);

            // Insert the two subtriangles that share edges E1 and E2.  A
            // potential side effect is that the triangle array is reallocated
            // to make room for the new triangles.  This will invalidate the
            // reference 'triangle' from the code above, but the index t into
            // the array is still correct.  A reallocation of the vertex array
            // might also occur.
            Insert(label1, label2, nextLabel);
            Insert(label2, label0, nextLabel);

            // Stitch the third subtriangle to the other subtriangles.
            MTTriangle& triangleN = mTriangles[t];
            int32_t subE1 = E(label1, nextLabel);
            int32_t subE2 = E(label0, nextLabel);
            MTEdge& subEdge1 = mEdges[subE1];
            MTEdge& subEdge2 = mEdges[subE2];
            AttachTriangleToEdge(t, triangleN, 1, subE1, subEdge1);
            AttachTriangleToEdge(t, triangleN, 2, subE2, subEdge2);
        }
        return true;
    }

    bool SubdivideEdge(int32_t label0, int32_t label1, int32_t& nextLabel)
    {
        int32_t e = E(label0, label1);
        if (e == -1)
        {
            return false;
        }

        if (mVMap.find(nextLabel) != mVMap.end())
        {
            // A vertex already exists with this label.
            return false;
        }

        // Split the triangles sharing the edge.
        MTEdge& edge = mEdges[e];
        int32_t t0 = edge.GetTriangle(0);
        int32_t t1 = edge.GetTriangle(1);
        int32_t t0v0, t0v1, t0v2, t1v0, t1v1, t1v2, t0e0, t0e1, t1e0, t1e1;
        if (t0 >= 0 && t1 == -1)
        {
            // The edge is shared only by T0.
            MTTriangle& triangle0 = mTriangles[t0];
            t0v0 = GetVLabel(triangle0.GetVertex(0));
            t0v1 = GetVLabel(triangle0.GetVertex(1));
            t0v2 = GetVLabel(triangle0.GetVertex(2));
            t0e0 = triangle0.GetEdge(0);
            t0e1 = triangle0.GetEdge(1);

            Remove(t0v0, t0v1, t0v2);
            if (t0e0 == e)
            {
                Insert(t0v0, nextLabel, t0v2);
                Insert(nextLabel, t0v1, t0v2);
            }
            else if (t0e1 == e)
            {
                Insert(t0v1, nextLabel, t0v0);
                Insert(nextLabel, t0v2, t0v0);
            }
            else
            {
                Insert(t0v2, nextLabel, t0v1);
                Insert(nextLabel, t0v0, t0v1);
            }
        }
        else if (t1 >= 0 && t0 == -1)
        {
            // The edge is shared only by T1.  The Remove(int32_t,int32_t,int32_t) call is
            // not factored outside the conditional statements to avoid
            // potential reallocation side effects that would invalidate the
            // reference 'triangle1'.
            MTTriangle& triangle1 = mTriangles[t1];
            t1v0 = GetVLabel(triangle1.GetVertex(0));
            t1v1 = GetVLabel(triangle1.GetVertex(1));
            t1v2 = GetVLabel(triangle1.GetVertex(2));
            t1e0 = triangle1.GetEdge(0);
            t1e1 = triangle1.GetEdge(1);

            Remove(t1v0, t1v1, t1v2);
            if (t1e0 == e)
            {
                Insert(t1v0, nextLabel, t1v2);
                Insert(nextLabel, t1v1, t1v2);
            }
            else if (t1e1 == e)
            {
                Insert(t1v1, nextLabel, t1v0);
                Insert(nextLabel, t1v2, t1v0);
            }
            else
            {
                Insert(t1v2, nextLabel, t1v1);
                Insert(nextLabel, t1v0, t1v1);
            }
        }
        else
        {
            LogAssert(t0 >= 0 && t1 >= 0, "Unexpected condition.");

            // The edge is shared both by T0 and T1.  The Remove(...) call
            // is not factored outside the conditional statements to avoid
            // potential reallocation side effects that would invalidate the
            /// references 'triangle0' and 'triangle1'.
            MTTriangle& triangle0 = mTriangles[t0];
            t0v0 = GetVLabel(triangle0.GetVertex(0));
            t0v1 = GetVLabel(triangle0.GetVertex(1));
            t0v2 = GetVLabel(triangle0.GetVertex(2));
            t0e0 = triangle0.GetEdge(0);
            t0e1 = triangle0.GetEdge(1);

            MTTriangle& triangle1 = mTriangles[t1];
            t1v0 = GetVLabel(triangle1.GetVertex(0));
            t1v1 = GetVLabel(triangle1.GetVertex(1));
            t1v2 = GetVLabel(triangle1.GetVertex(2));
            t1e0 = triangle1.GetEdge(0);
            t1e1 = triangle1.GetEdge(1);

            // Both triangles must be removed before the insertions to guarantee
            // that the common edge is deleted first from the mesh.
            Remove(t0v0, t0v1, t0v2);
            Remove(t1v0, t1v1, t1v2);

            if (t0e0 == e)
            {
                Insert(t0v0, nextLabel, t0v2);
                Insert(nextLabel, t0v1, t0v2);
            }
            else if (t0e1 == e)
            {
                Insert(t0v1, nextLabel, t0v0);
                Insert(nextLabel, t0v2, t0v0);
            }
            else
            {
                Insert(t0v2, nextLabel, t0v1);
                Insert(nextLabel, t0v0, t0v1);
            }

            if (t1e0 == e)
            {
                Insert(t1v0, nextLabel, t1v2);
                Insert(nextLabel, t1v1, t1v2);
            }
            else if (t1e1 == e)
            {
                Insert(t1v1, nextLabel, t1v0);
                Insert(nextLabel, t1v2, t1v0);
            }
            else
            {
                Insert(t1v2, nextLabel, t1v1);
                Insert(nextLabel, t1v0, t1v1);
            }
        }

        ++nextLabel;
        return true;
    }

    virtual void Print(std::ofstream& output) const
    {
        int32_t v, e, t;

        // Print the vertex information.
        int32_t const numVertices = mVertices.GetNumElements();
        output << "vertex quantity = " << numVertices << std::endl;
        for (v = 0; v < numVertices; ++v)
        {
            const MTVertex& vertex = mVertices[v];

            output << "vertex<" << v << ">" << std::endl;
            output << "    l: " << vertex.GetLabel() << std::endl;
            output << "    e: ";
            const int32_t numEdges = vertex.GetNumEdges();
            for (e = 0; e < numEdges; ++e)
            {
                output << vertex.GetEdge(e) << ' ';
            }
            output << std::endl;
            output << "    t: ";
            const int32_t numTriangles = vertex.GetNumTriangles();
            for (t = 0; t < numTriangles; ++t)
            {
                output << vertex.GetTriangle(t) << ' ';
            }
            output << std::endl;
        }
        output << std::endl;

        // Print the edge information.
        int32_t const numEdges = mEdges.GetNumElements();
        output << "edge quantity = " << numEdges << std::endl;
        for (e = 0; e < numEdges; ++e)
        {
            const MTEdge& edge = mEdges[e];

            output << "edge<" << e << ">" << std::endl;
            output << "    v: " << edge.GetVertex(0) << ' ' << edge.GetVertex(1)
                << std::endl;
            output << "    t: " << edge.GetTriangle(0) << ' '
                << edge.GetTriangle(1) << std::endl;
        }
        output << std::endl;

        // Print the triangle information.
        int32_t const numTriangles = mTriangles.GetNumElements();
        output << "triangle quantity = " << numTriangles << std::endl;
        for (t = 0; t < numTriangles; ++t)
        {
            const MTTriangle& triangle = mTriangles[t];

            output << "triangle<" << t << ">" << std::endl;
            output << "    v: "
                << triangle.GetVertex(0) << ' '
                << triangle.GetVertex(1) << ' '
                << triangle.GetVertex(2) << std::endl;
            output << "    e: "
                << triangle.GetEdge(0) << ' '
                << triangle.GetEdge(1) << ' '
                << triangle.GetEdge(2) << std::endl;
            output << "    a: "
                << triangle.GetAdjacent(0) << ' '
                << triangle.GetAdjacent(1) << ' '
                << triangle.GetAdjacent(2) << std::endl;
        }
        output << std::endl;
    }

    virtual bool Print(std::string const& filename) const
    {
        std::ofstream output(filename);
        if (!output)
        {
            return false;
        }

        Print(output);
        return true;
    }

protected:
    typedef std::map<int32_t, int32_t> VMap;
    typedef std::map<gte::EdgeKey<false>, int32_t> EMap;
    typedef std::map<gte::TriangleKey<false>, int32_t> TMap;

    void AttachTriangleToEdge(int32_t t, MTTriangle& triangle, int32_t i, int32_t e, MTEdge& edge)
    {
        if (edge.GetTriangle(0) == -1)
        {
            edge.SetTriangle(0, t);
        }
        else
        {
            int32_t a = edge.GetTriangle(0);
            MTTriangle& adjacent = mTriangles[a];
            triangle.SetAdjacent(i, a);
            for (int32_t j = 0; j < 3; ++j)
            {
                if (adjacent.GetEdge(j) == e)
                {
                    adjacent.SetAdjacent(j, t);
                    break;
                }
            }

            if (edge.GetTriangle(1) == -1)
            {
                edge.SetTriangle(1, t);
            }
            else
            {
                LogError("The mesh is not manifold.");
            }
        }

        triangle.SetEdge(i, e);
    }

    int32_t InsertVertex(int32_t label)
    {
        int32_t v;

        auto iter = mVMap.find(label);
        if (iter != mVMap.end())
        {
            // The vertex already exists.
            v = iter->second;
        }
        else
        {
            // Create a new vertex.
            v = mVertices.Append(MTVertex(label));
            mVMap.insert(std::make_pair(label, v));
        }

        return v;
    }

    int32_t InsertEdge(int32_t label0, int32_t label1)
    {
        gte::EdgeKey<false> edge(label0, label1);
        int32_t e;

        auto iter = mEMap.find(edge);
        if (iter != mEMap.end())
        {
            // The edge already exists.
            e = iter->second;
        }
        else
        {
            // Create a new edge.
            e = mEdges.Append(MTEdge(mInitialELabel));
            mEMap.insert(std::make_pair(edge, e));
        }

        return e;
    }

    int32_t InsertTriangle(int32_t label0, int32_t label1, int32_t label2)
    {
        gte::TriangleKey<false> triangle(label0, label1, label2);
        int32_t t;

        auto iter = mTMap.find(triangle);
        if (iter != mTMap.end())
        {
            // The triangle already exists.
            t = -1;
        }
        else
        {
            // create new triangle
            t = mTriangles.Append(MTTriangle(mInitialTLabel));
            mTMap.insert(std::make_pair(triangle, t));
        }

        return t;
    }

    void DetachTriangleFromEdge(int32_t t, MTTriangle& triangle, int32_t i, int32_t e, MTEdge& edge)
    {
        // This function leaves T only partially complete.  The edge E is no
        // longer referenced by T, even though the vertices of T reference the
        // end points of E.  If T has an adjacent triangle A that shares E,
        // then A is a complete triangle.

        if (edge.GetTriangle(0) == t)
        {
            int32_t a = edge.GetTriangle(1);
            if (a != -1)
            {
                // T and TAdj share E, update adjacency information for both
                MTTriangle& adjacent = mTriangles[a];
                for (int32_t j = 0; j < 3; ++j)
                {
                    if (adjacent.GetEdge(j) == e)
                    {
                        adjacent.SetAdjacent(j, -1);
                        break;
                    }
                }
            }
            edge.SetTriangle(0, a);
        }
        else if (edge.GetTriangle(1) == t)
        {
            // T and TAdj share E, update adjacency information for both
            MTTriangle& adjacent = mTriangles[edge.GetTriangle(0)];
            for (int32_t j = 0; j < 3; ++j)
            {
                if (adjacent.GetEdge(j) == e)
                {
                    adjacent.SetAdjacent(j, -1);
                    break;
                }
            }
        }
        else
        {
            // Should not get here.  The specified edge must share the input
            // triangle.
            LogError("Unexpected condition.");
        }

        edge.SetTriangle(1, -1);
        triangle.SetEdge(i, -1);
        triangle.SetAdjacent(i, -1);
    }

    void RemoveVertex(int32_t label)
    {
        // Get array location of vertex.
        auto iter = mVMap.find(label);
        if (iter == mVMap.end())
        {
            LogError("Vertex does not exist.");
            return;
        }
        int32_t v = iter->second;

        // Remove the vertex from the array and from the map.
        int32_t vOld, vNew;
        mVertices.RemoveAt(v, &vOld, &vNew);
        mVMap.erase(iter);

        if (vNew >= 0)
        {
            // The vertex at the end of the array moved into the slot vacated
            // by the deleted vertex.  Update all the components sharing the
            // moved vertex.
            MTVertex& vertex = mVertices[vNew];

            // Inform edges about location change.
            for (int32_t e = 0; e < vertex.GetNumEdges(); ++e)
            {
                MTEdge& edge = mEdges[vertex.GetEdge(e)];
                edge.ReplaceVertex(vOld, vNew);
            }

            // Inform triangles about location change.
            for (int32_t t = 0; t < vertex.GetNumTriangles(); ++t)
            {
                MTTriangle& triangle = mTriangles[vertex.GetTriangle(t)];
                triangle.ReplaceVertex(vOld, vNew);
            }

            iter = mVMap.find(vertex.GetLabel());
            LogAssert(iter != mVMap.end(), "Vertex does not exist.");
            iter->second = vNew;
        }
    }

    void RemoveEdge(int32_t label0, int32_t label1)
    {
        // Get array location of edge.
        auto iter = mEMap.find(gte::EdgeKey<false>(label0, label1));
        if (iter == mEMap.end())
        {
            LogError("Edge does not exist.");
            return;
        }
        int32_t e = iter->second;

        // Remove the edge from the array and from the map.
        int32_t eOld, eNew;
        mEdges.RemoveAt(e, &eOld, &eNew);
        mEMap.erase(iter);

        if (eNew >= 0)
        {
            // The edge at the end of the array moved into the slot vacated by
            // the deleted edge.  Update all the components sharing the moved
            // edge.
            MTEdge& edge = mEdges[eNew];

            // Inform vertices about location change.
            MTVertex& vertex0 = mVertices[edge.GetVertex(0)];
            MTVertex& vertex1 = mVertices[edge.GetVertex(1)];
            vertex0.ReplaceEdge(eOld, eNew);
            vertex1.ReplaceEdge(eOld, eNew);

            // Inform triangles about location change.
            for (int32_t t = 0; t < 2; ++t)
            {
                int32_t tIndex = edge.GetTriangle(t);
                if (tIndex != -1)
                {
                    MTTriangle& triangle = mTriangles[tIndex];
                    triangle.ReplaceEdge(eOld, eNew);
                }
            }

            iter = mEMap.find(gte::EdgeKey<false>(vertex0.GetLabel(), vertex1.GetLabel()));
            LogAssert(iter != mEMap.end(), "Edge does not exist.");
            iter->second = eNew;
        }
    }

    void RemoveTriangle(int32_t label0, int32_t label1, int32_t label2)
    {
        // Get array location of triangle.
        auto iter = mTMap.find(gte::TriangleKey<false>(label0, label1, label2));
        if (iter == mTMap.end())
        {
            LogError("Triangle does not exist.");
            return;
        }
        int32_t t = iter->second;

        // Remove the triangle from the array and from the map.
        int32_t tOld, tNew;
        mTriangles.RemoveAt(t, &tOld, &tNew);
        mTMap.erase(iter);

        if (tNew >= 0)
        {
            // The triangle at the end of the array moved into the slot
            // vacated by the deleted triangle.  Update all the components
            // sharing the moved triangle.
            MTTriangle& triangle = mTriangles[tNew];

            // Inform vertices about location change.
            MTVertex& vertex0 = mVertices[triangle.GetVertex(0)];
            MTVertex& vertex1 = mVertices[triangle.GetVertex(1)];
            MTVertex& vertex2 = mVertices[triangle.GetVertex(2)];
            vertex0.ReplaceTriangle(tOld, tNew);
            vertex1.ReplaceTriangle(tOld, tNew);
            vertex2.ReplaceTriangle(tOld, tNew);

            // Inform edges about location change.
            for (int32_t e = 0; e < 3; ++e)
            {
                MTEdge& edge = mEdges[triangle.GetEdge(e)];
                edge.ReplaceTriangle(tOld, tNew);
            }

            // Inform adjacents about location change.
            for (int32_t a = 0; a < 3; ++a)
            {
                int32_t aIndex = triangle.GetAdjacent(a);
                if (aIndex != -1)
                {
                    MTTriangle& adjacent = mTriangles[aIndex];
                    adjacent.ReplaceAdjacent(tOld, tNew);
                }
            }

            iter = mTMap.find(
                gte::TriangleKey<false>(vertex0.GetLabel(), vertex1.GetLabel(), vertex2.GetLabel()));
            LogAssert(iter != mTMap.end(), "Triangle does not exist.");
            iter->second = tNew;
        }
    }

    UnorderedSet<MTVertex> mVertices;
    UnorderedSet<MTEdge> mEdges;
    UnorderedSet<MTTriangle> mTriangles;
    int32_t mInitialELabel;
    int32_t mInitialTLabel;
    VMap mVMap;
    EMap mEMap;
    TMap mTMap;
};
