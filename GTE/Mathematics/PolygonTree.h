// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 5.3.2020.10.26

#pragma once

#include <Mathematics/Vector2.h>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

namespace gte
{
    // These classes are used by class TriangulateEC (triangulation based on
    // ear clipping) and class TriangulateCDT (triangulation based on
    // Constrained Delaunay triangulation). The PolygonTree class used to be
    // the nested class Tree in those classes, but it has been factored out to
    // allow applications to use either triangulator without having to
    // duplicate the trees.
    //
    // NOTE: The polygon member does not duplicate endpoints. For example,
    // if P[] are the point locations and the polygon is a triangle with
    // counterclockwise ordering, <P[i0],P[i1],P[i2]>, then
    // polygon = {i0,i1,i2}. The implication is that there are 3 directed
    // edges: {P[i0],P[i1]}, {P[i1],P[i2]} and {P[i2],P[i0].
    //
    // Eventually, the PolygonTreeEx struct will replace PolygonTree because
    //   1. The algorithms can be rewritten not to depend on the alternating
    //      winding order between parent and child.
    //   2. The triangulation is explicitly stored in the tree nodes and can
    //      support point-in-polygon tree queries (In the tree? Which polygon
    //      contains the point?).
    //   3. The polygon trees can be built not to use std::shared_ptr, making
    //      the trees more compact by using std::vector<PolygonTree> vpt. The
    //      ordering of the tree nodes must be that implied by a breadth-first
    //      search.

    // A tree of nested polygons. The root node corresponds to an outer
    // polygon. The children of the root correspond to inner polygons,
    // which polygons strictly contained in the outer polygon. Each inner
    // polygon may itself contain an outer polygon which in turn can
    // contain inner polygons, thus leading to a hierarchy of polygons.
    // The outer polygons have vertices listed in counterclockwise order.
    // The inner polygons have vertices listed in clockwise order.
    class PolygonTree
    {
    public:
        PolygonTree()
            :
            polygon{},
            child{}
        {
        }

        std::vector<int> polygon;
        std::vector<std::shared_ptr<PolygonTree>> child;
    };

    // A tree of nested polygons with extra information about the polygon.
    // The tree can be stored as: std::vector<PolygonTree> tree(numNodes).
    // The point locations are specified separately to the triangulators.
    //
    // The chirality (winding ordering of the polygon) is set to +1 for a
    // counterclockwise-ordered polygon or -1 for a clockwise-oriented
    // polygon.
    //
    // The triangulation is computed by the triangulators and explicitly
    // stored per tree node.
    //
    // The element node[0] is the root of the tree with node[0].parent = -1.
    // If node[0] has C children, then node[0].minChild = 1 and
    // node[0].supChild = 1 + C. Generally, node[i] is a node with parent
    // node[p], where p = node[i].parent, and children node[c], where
    // node[i].minChild <= c < node[i].supChild. If node[i].minChild >=
    // node[i].supChild, the node has no children.
    class PolygonTreeEx
    {
    public:
        class Node
        {
        public:
            Node()
                :
                polygon{},
                chirality(0),
                triangulation{},
                self(0),
                parent(0),
                minChild(0),
                supChild(0)
            {
            }

            std::vector<int> polygon;
            int64_t chirality;
            std::vector<std::array<int, 3>> triangulation;
            size_t self, parent, minChild, supChild;
        };

        // Search the polygon tree for the triangle that contains 'test'.
        // If there is such a triangle, the returned pair (nIndex,tIndex)
        // states that the triangle is nodes[nIndex].triangulation[tIndex].
        // If there is no such triangle, the returned pari is (smax,smax)
        // where smax = std::numeric_limits<size_t>::max().
        template <typename T>
        std::pair<size_t, size_t> GetContainingTriangle(Vector2<T> const& test,
            Vector2<T> const* points)
        {
            size_t constexpr smax = std::numeric_limits<size_t>::max();
            std::pair<size_t, size_t> result = std::make_pair(smax, smax);
            GetContainingTriangleRecurse(0, test, points, result);
            return result;
        }

        // The nodes of the polygon tree, organized based on a breadth-first
        // search of the tree.
        std::vector<Node> nodes;

        // These members support TriangulateCDT at the moment.

        // The triangles inside the polygon tree.
        std::vector<std::array<int, 3>> insideTriangles;

        // The triangles inside the convex hull of the Delaunay triangles but
        // outside the polygon tree.
        std::vector<std::array<int, 3>> outsideTriangles;

        // All the triangles, the union of the inside and outside triangles.
        std::vector<std::array<int, 3>> allTriangles;

    private:
        template <typename T>
        void GetContainingTriangleRecurse(size_t nIndex, Vector2<T> const& test,
            Vector2<T> const* points, std::pair<size_t, size_t>& result)
        {
            auto const& node = nodes[nIndex];
            for (size_t c = node.minChild; c < node.supChild; ++c)
            {
                GetContainingTriangleRecurse(c, test, points, result);
                if (result.first != std::numeric_limits<size_t>::max())
                {
                    return;
                }
            }

            for (size_t tIndex = 0; tIndex < node.triangulation.size(); ++tIndex)
            {
                if (PointInTriangle(test, node.chirality, node.triangulation[tIndex], points))
                {
                    result = std::make_pair(nIndex, tIndex);
                    return;
                }
            }
        }

        template <typename T>
        bool PointInTriangle(Vector2<T> const& test, int64_t chirality,
            std::array<int, 3> const& triangle, Vector2<T> const* points)
        {
            T const zero = static_cast<T>(0);
            T const sign = static_cast<T>(chirality);
            for (int i1 = 0, i0 = 2; i1 < 3; i0 = i1++)
            {
                T nx = points[triangle[i1]][1] - points[triangle[i0]][1];
                T ny = points[triangle[i0]][0] - points[triangle[i1]][0];
                T dx = test[0] - points[triangle[i0]][0];
                T dy = test[1] - points[triangle[i0]][1];
                T sdot = sign * (nx * dx + ny * dy);
                if (sdot > zero)
                {
                    return false;
                }
            }
            return true;
        }
    };
}
