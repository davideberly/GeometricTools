// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/ConstrainedDelaunay2.h>
#include <Mathematics/ContPointInPolygon2.h>
#include <memory>
#include <queue>

// The triangulation is based on constrained Delaunay triangulations (CDT),
// which does not use divisions, so ComputeType may be chosen using BSNumber.
// The input constraints are relaxed compared to TriangulateEC; specifically,
// the inner polygons are allowed to share vertices with the outer polygons.
// The CDT produces a triangulation of the convex hull of the input, which
// includes triangles outside the top-level outer polygon and inside the
// inner polygons.  Only the triangles relevant to the input are returned
// via the 'std::vector<int>& triangles', but the other triangles are
// retained so that you can perform linear walks in search of points inside
// the original polygon (nested polygon, tree of nested polygons).  This is
// useful, for example, when subsampling the polygon triangles for
// interpolation of function data specified at the vertices.  A linear walk
// does not work for a mesh consisting only of the polygon triangles, but
// with the additional triangles, the walk can navigate through holes in
// the polygon to find the containing triangle of the specified point.

namespace gte
{
    template <typename InputType, typename ComputeType>
    class TriangulateCDT
    {
    public:
        // The class is a functor to support triangulating multiple polygons
        // that share vertices in a collection of points.  The interpretation
        // of 'numPoints' and 'points' is described before each operator()
        // function.  Preconditions are numPoints >= 3 and points is a nonnull
        // pointer to an array of at least numPoints elements.  If the
        // preconditions are satisfied, then operator() functions will return
        // 'true'; otherwise, they return 'false'.
        TriangulateCDT(int numPoints, Vector2<InputType> const* points)
            :
            mNumPoints(numPoints),
            mPoints(points)
        {
            LogAssert(mNumPoints >= 3 && mPoints != nullptr, "Invalid input.");
        }

        TriangulateCDT(std::vector<Vector2<InputType>> const& points)
            :
            mNumPoints(static_cast<int>(points.size())),
            mPoints(points.data())
        {
            LogAssert(mNumPoints >= 3 && mPoints != nullptr, "Invalid input.");
        }

        // The triangles of the polygon triangulation.
        inline std::vector<std::array<int, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        // The triangles inside the convex hull of the points but outside the
        // triangulation.
        inline std::vector<std::array<int, 3>> const& GetOutsideTriangles() const
        {
            return mOutsideTriangles;
        }

        // The triangles of the convex hull of the inputs to the constructor.
        inline std::vector<std::array<int, 3>> const& GetAllTriangles() const
        {
            return mAllTriangles;
        }

        // The classification of whether a triangle is part of the
        // triangulation or outside the triangulation.  These may be used in
        // conjunction with the array returned by GetAllTriangles().
        inline std::vector<bool> const& GetIsInside() const
        {
            return mIsInside;
        }

        inline bool IsInside(int triIndex) const
        {
            if (0 <= triIndex && triIndex < static_cast<int>(mIsInside.size()))
            {
                return mIsInside[triIndex];
            }
            else
            {
                return false;
            }
        }

        inline bool IsOutside(int triIndex) const
        {
            if (0 <= triIndex && triIndex < static_cast<int>(mIsInside.size()))
            {
                return !mIsInside[triIndex];
            }
            else
            {
                return false;
            }
        }

        // The outer polygons have counterclockwise ordered vertices.  The
        // inner polygons have clockwise ordered vertices.
        typedef std::vector<int> Polygon;

        // The input 'points' represents an array of vertices for a simple
        // polygon. The vertices are points[0] through points[numPoints-1] and
        // are listed in counterclockwise order.
        bool operator()()
        {
            if (mPoints)
            {
                auto tree = std::make_shared<Tree>();
                tree->polygon.resize(mNumPoints);
                for (int i = 0; i < mNumPoints; ++i)
                {
                    tree->polygon[i] = i;
                }

                return operator()(tree);
            }
            return false;
        }

        // The input 'points' represents an array of vertices that contains
        // the vertices of a simple polygon.
        bool operator()(Polygon const& polygon)
        {
            if (mPoints)
            {
                auto tree = std::make_shared<Tree>();
                tree->polygon = polygon;

                return operator()(tree);
            }
            return false;
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for two simple polygons, an outer polygon and an inner
        // polygon.  The inner polygon must be strictly inside the outer
        // polygon.
        bool operator()(Polygon const& outer, Polygon const& inner)
        {
            if (mPoints)
            {
                auto otree = std::make_shared<Tree>();
                otree->polygon = outer;
                otree->child.resize(1);

                auto itree = std::make_shared<Tree>();
                itree->polygon = inner;
                otree->child[0] = itree;

                return operator()(otree);
            }
            return false;
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for multiple simple polygons, an outer polygon and one or
        // more inner polygons.  The inner polygons must be nonoverlapping and
        // strictly inside the outer polygon.
        bool operator()(Polygon const& outer, std::vector<Polygon> const& inners)
        {
            if (mPoints)
            {
                auto otree = std::make_shared<Tree>();
                otree->polygon = outer;
                otree->child.resize(inners.size());

                std::vector<std::shared_ptr<Tree>> itree(inners.size());
                for (size_t i = 0; i < inners.size(); ++i)
                {
                    itree[i] = std::make_shared<Tree>();
                    itree[i]->polygon = inners[i];
                    otree->child[i] = itree[i];
                }

                return operator()(otree);
            }
            return false;
        }

        // A tree of nested polygons.  The root node corresponds to an outer
        // polygon.  The children of the root correspond to inner polygons,
        // which are nonoverlapping polygons strictly contained in the outer
        // polygon.  Each inner polygon may itself contain an outer polygon,
        // thus leading to a hierarchy of polygons.  The outer polygons have
        // vertices listed in counterclockwise order.  The inner polygons have
        // vertices listed in clockwise order.
        class Tree
        {
        public:
            Polygon polygon;
            std::vector<std::shared_ptr<Tree>> child;
        };

        // The input 'positions' is a shared array of vertices that contains
        // the vertices for multiple simple polygons in a tree of polygons.
        bool operator()(std::shared_ptr<Tree> const& tree)
        {
            if (mPoints)
            {
                std::map<std::shared_ptr<Tree>, int> offsets;
                int numPoints = GetNumPointsAndOffsets(tree, offsets);
                std::vector<Vector2<InputType>> points(numPoints);
                PackPoints(tree, points);

                if (TriangulatePacked(numPoints, &points[0], tree, offsets))
                {
                    int numTriangles = static_cast<int>(mAllTriangles.size());
                    for (int t = 0; t < numTriangles; ++t)
                    {
                        auto& tri = mAllTriangles[t];
                        for (int j = 0; j < 3; ++j)
                        {
                            LookupIndex(tree, tri[j], offsets);
                        }

                        if (mIsInside[t])
                        {
                            mTriangles.push_back(tri);
                        }
                        else
                        {
                            mOutsideTriangles.push_back(tri);
                        }
                    }
                    return true;
                }
            }

            return false;
        }

    private:
        // Triangulate the points referenced by an operator(...) query.  The
        // mAllTriangles and mIsInside are populated by this function, but the
        // indices of mAllTriangles are relative to the packed 'points'.
        // After the call, the indices need to be mapped back to the original
        // set provided by the input arrays to operator(...).  The mTriangles
        // and mOutsideTriangles are generated after the call by examining
        // mAllTriangles and mIsInside.
        bool TriangulatePacked(int numPoints, Vector2<InputType> const* points,
            std::shared_ptr<Tree> const& tree,
            std::map<std::shared_ptr<Tree>, int> const& offsets)
        {
            mTriangles.clear();
            mOutsideTriangles.clear();
            mAllTriangles.clear();
            mIsInside.clear();

            mConstrainedDelaunay(numPoints, points, static_cast<InputType>(0));
            InsertEdges(tree);

            ComputeType half = static_cast<ComputeType>(0.5);
            ComputeType fourth = static_cast<ComputeType>(0.25);
            auto const& query = mConstrainedDelaunay.GetQuery();
            auto const* ctPoints = query.GetVertices();
            int numTriangles = mConstrainedDelaunay.GetNumTriangles();
            int const* indices = &mConstrainedDelaunay.GetIndices()[0];
            mIsInside.resize(numTriangles);
            for (int t = 0; t < numTriangles; ++t)
            {
                int v0 = *indices++;
                int v1 = *indices++;
                int v2 = *indices++;
                auto ctInside = fourth * ctPoints[v0] + half * ctPoints[v1] + fourth * ctPoints[v2];
                mIsInside[t] = IsInside(tree, ctPoints, ctInside, offsets);
                mAllTriangles.push_back( { v0, v1, v2 } );
            }
            return true;
        }

        int GetNumPointsAndOffsets(std::shared_ptr<Tree> const& tree,
            std::map<std::shared_ptr<Tree>, int>& offsets) const
        {
            int numPoints = 0;
            std::queue<std::shared_ptr<Tree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                std::shared_ptr<Tree> outer = treeQueue.front();
                treeQueue.pop();
                offsets.insert(std::make_pair(outer, numPoints));
                numPoints += static_cast<int>(outer->polygon.size());

                int numChildren = static_cast<int>(outer->child.size());
                for (int c = 0; c < numChildren; ++c)
                {
                    std::shared_ptr<Tree> inner = outer->child[c];
                    offsets.insert(std::make_pair(inner, numPoints));
                    numPoints += static_cast<int>(inner->polygon.size());

                    int numGrandChildren = static_cast<int>(inner->child.size());
                    for (int g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }
            return numPoints;
        }

        void PackPoints(std::shared_ptr<Tree> const& tree,
            std::vector<Vector2<InputType>>& points)
        {
            int numPoints = 0;
            std::queue<std::shared_ptr<Tree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                std::shared_ptr<Tree> outer = treeQueue.front();
                treeQueue.pop();
                int const numOuterIndices = static_cast<int>(outer->polygon.size());
                int const* outerIndices = outer->polygon.data();
                for (int i = 0; i < numOuterIndices; ++i)
                {
                    points[numPoints++] = mPoints[outerIndices[i]];
                }

                int numChildren = static_cast<int>(outer->child.size());
                for (int c = 0; c < numChildren; ++c)
                {
                    std::shared_ptr<Tree> inner = outer->child[c];
                    int const numInnerIndices = static_cast<int>(inner->polygon.size());
                    int const* innerIndices = inner->polygon.data();
                    for (int i = 0; i < numInnerIndices; ++i)
                    {
                        points[numPoints++] = mPoints[innerIndices[i]];
                    }

                    int numGrandChildren = static_cast<int>(inner->child.size());
                    for (int g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }
        }

        bool InsertEdges(std::shared_ptr<Tree> const& tree)
        {
            int numPoints = 0;
            std::array<int, 2> edge;
            std::vector<int> ignoreOutEdge;
            std::queue<std::shared_ptr<Tree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                auto outer = treeQueue.front();
                treeQueue.pop();
                int numOuter = static_cast<int>(outer->polygon.size());
                for (int i0 = numOuter - 1, i1 = 0; i1 < numOuter; i0 = i1++)
                {
                    edge[0] = numPoints + i0;
                    edge[1] = numPoints + i1;
                    if (!mConstrainedDelaunay.Insert(edge, ignoreOutEdge))
                    {
                        return false;
                    }
                }
                numPoints += numOuter;

                int numChildren = static_cast<int>(outer->child.size());
                for (int c = 0; c < numChildren; ++c)
                {
                    auto inner = outer->child[c];
                    int numInner = static_cast<int>(inner->polygon.size());
                    for (int i0 = numInner - 1, i1 = 0; i1 < numInner; i0 = i1++)
                    {
                        edge[0] = numPoints + i0;
                        edge[1] = numPoints + i1;
                        if (!mConstrainedDelaunay.Insert(edge, ignoreOutEdge))
                        {
                            return false;
                        }
                    }
                    numPoints += numInner;

                    int numGrandChildren = static_cast<int>(inner->child.size());
                    for (int g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }
            return true;
        }

        void LookupIndex(std::shared_ptr<Tree> const& tree, int& v,
            std::map<std::shared_ptr<Tree>, int> const& offsets) const
        {
            std::queue<std::shared_ptr<Tree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                auto outer = treeQueue.front();
                treeQueue.pop();
                int const numOuterIndices = static_cast<int>(outer->polygon.size());
                int const* outerIndices = outer->polygon.data();
                int offset = offsets.find(outer)->second;
                if (v < offset + numOuterIndices)
                {
                    v = outerIndices[v - offset];
                    return;
                }

                int numChildren = static_cast<int>(outer->child.size());
                for (int c = 0; c < numChildren; ++c)
                {
                    auto inner = outer->child[c];
                    int const numInnerIndices = static_cast<int>(inner->polygon.size());
                    int const* innerIndices = inner->polygon.data();
                    offset = offsets.find(inner)->second;
                    if (v < offset + numInnerIndices)
                    {
                        v = innerIndices[v - offset];
                        return;
                    }

                    int numGrandChildren = static_cast<int>(inner->child.size());
                    for (int g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }
        }

        bool IsInside(std::shared_ptr<Tree> const& tree, Vector2<ComputeType> const* points,
            Vector2<ComputeType> const& test,
            std::map<std::shared_ptr<Tree>, int> const& offsets) const
        {
            std::queue<std::shared_ptr<Tree>> treeQueue;
            treeQueue.push(tree);
            while (treeQueue.size() > 0)
            {
                auto outer = treeQueue.front();
                treeQueue.pop();
                int const numOuterIndices = static_cast<int>(outer->polygon.size());
                int offset = offsets.find(outer)->second;
                PointInPolygon2<ComputeType> piOuter(numOuterIndices, points + offset);
                if (piOuter.Contains(test))
                {
                    int numChildren = static_cast<int>(outer->child.size());
                    int c;
                    for (c = 0; c < numChildren; ++c)
                    {
                        auto inner = outer->child[c];
                        int const numInnerIndices = static_cast<int>(inner->polygon.size());
                        offset = offsets.find(inner)->second;
                        PointInPolygon2<ComputeType> piInner(numInnerIndices, points + offset);
                        if (piInner.Contains(test))
                        {
                            int numGrandChildren = static_cast<int>(inner->child.size());
                            for (int g = 0; g < numGrandChildren; ++g)
                            {
                                treeQueue.push(inner->child[g]);
                            }
                            break;
                        }
                    }
                    if (c == numChildren)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        // The input polygon.
        int mNumPoints;
        Vector2<InputType> const* mPoints;

        // The output triangulation and those triangle inside the hull of the
        // points but outside the triangulation.
        std::vector<std::array<int, 3>> mTriangles;
        std::vector<std::array<int, 3>> mOutsideTriangles;
        std::vector<std::array<int, 3>> mAllTriangles;
        std::vector<bool> mIsInside;

        ConstrainedDelaunay2<InputType, ComputeType> mConstrainedDelaunay;
    };
}
