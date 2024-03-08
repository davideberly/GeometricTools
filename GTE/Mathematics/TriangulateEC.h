// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.12.30

#pragma once

// Triangulate polygons using ear clipping. The algorithm is described in
// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
// The algorithm for processing nested polygons involves a division, so the
// ComputeType must be rational-based, say, BSRational. If you process only
// triangles that are simple, you may use BSNumber for the ComputeType.

#include <Mathematics/Logger.h>
#include <Mathematics/PolygonTree.h>
#include <Mathematics/PrimalQuery2.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

namespace gte
{
    template <typename InputType, typename ComputeType>
    class TriangulateEC
    {
    public:
        // The fundamental problem is to compute the triangulation of a
        // polygon tree. The outer polygons have counterclockwise ordered
        // vertices. The inner polygons have clockwise ordered vertices.
        typedef std::vector<int32_t> Polygon;

        // The class is a functor to support triangulating multiple polygons
        // that share vertices in a collection of points. The interpretation
        // of 'numPoints' and 'points' is described before each operator()
        // function. Preconditions are numPoints >= 3 and points is a nonnull
        // pointer to an array of at least numPoints elements. If they are not
        // satisfied, an exception is thrown.
        TriangulateEC(int32_t numPoints, Vector2<InputType> const* points)
            :
            mNumPoints(numPoints),
            mPoints(points),
            mTriangles{},
            mComputePoints{},
            mConverted{},
            mQuery{},
            mVertexList{}
        {
            LogAssert(
                numPoints >= 3 && points != nullptr,
                "Invalid input.");

            mComputePoints.resize(mNumPoints);
            mConverted.resize(mNumPoints);
            std::fill(mConverted.begin(), mConverted.end(), false);
            mQuery.Set(mNumPoints, mComputePoints.data());
        }

        TriangulateEC(std::vector<Vector2<InputType>> const& points)
            :
            TriangulateEC(static_cast<int32_t>(points.size()), points.data())
        {
        }

        // Access the triangulation after each operator() call.
        inline std::vector<std::array<int32_t, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        // The input 'points' represents an array of vertices for a simple
        // polygon. The vertices are points[0] through points[numPoints-1] and
        // are listed in counterclockwise order.
        void operator()()
        {
            mTriangles.clear();
            Polygon polygon(mNumPoints);
            std::iota(polygon.begin(), polygon.end(), 0);
            operator()(polygon);
        }

        // The input 'points' represents an array of vertices that contains
        // the vertices of a simple polygon.
        void operator()(Polygon const& polygon)
        {
            mTriangles.clear();

            // Convert InputType polygon vertices to ComputeType, the latter
            // type presumably an exact rational type.
            ConvertPoints(polygon);

            // Triangulate the simple polygon using ear clipping.
            mVertexList.DoEarClipping(polygon, mComputePoints, mQuery, mTriangles);
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for two simple polygons, an outer polygon and an inner
        // polygon. The inner polygon must be strictly inside the outer
        // polygon.
        void operator()(Polygon const& outer, Polygon const& inner)
        {
            mTriangles.clear();

            // Convert InputType polygon vertices to ComputeType, the latter
            // type presumably an exact rational type.
            ConvertPoints(outer, inner);

            // Combine the inner and outer polygon into a pseudosimple
            // polygon.
            Polygon combined{};
            CombineSingle(outer, inner, combined);

            // Triangulate the pseudosimple polygon using ear clipping.
            mVertexList.DoEarClipping(combined, mComputePoints, mQuery, mTriangles);
        }

        // The input 'points' is a shared array of vertices that contains the
        // vertices for multiple simple polygons, an outer polygon and one or
        // more inner polygons. The inner polygons must be nonoverlapping and
        // strictly inside the outer polygon.
        void operator()(Polygon const& outer, std::vector<Polygon> const& inners)
        {
            mTriangles.clear();

            // Convert InputType polygon vertices to ComputeType, the latter
            // type presumably an exact rational type.
            ConvertPoints(outer, inners);

            // Combine the outer polygon and the inner polygons into a
            // pseudosimple polygon using repeated calls to CombineSimple.
            Polygon combined{};
            CombineMultiple(outer, inners, combined);

            // Triangulate the pseudosimple polygon using ear clipping.
            mVertexList.DoEarClipping(combined, mComputePoints, mQuery, mTriangles);
        }

        // The input 'positions' is a shared array of vertices that contains
        // the vertices for multiple simple polygons in a tree of polygons.
        void operator()(std::shared_ptr<PolygonTree> const& tree)
        {
            mTriangles.clear();

            // Convert InputType polygon vertices to ComputeType, the latter
            // type presumably an exact rational type.
            ConvertPoints(tree);

            std::queue<std::shared_ptr<PolygonTree>> treeQueue{};
            treeQueue.push(tree);
            while (!treeQueue.empty())
            {
                std::shared_ptr<PolygonTree> outer = treeQueue.front();
                treeQueue.pop();

                int32_t numChildren = static_cast<int32_t>(outer->child.size());
                if (numChildren == 0)
                {
                    // The outer polygon is a simple polygon that has no
                    // nested inner polygons. Triangulate the pseudosimple
                    // polygon using ear clipping.
                    std::vector<std::array<int32_t, 3>> combinedTriangles{};
                    mVertexList.DoEarClipping(outer->polygon, mComputePoints, mQuery, combinedTriangles);
                    mTriangles.insert(mTriangles.end(), combinedTriangles.begin(), combinedTriangles.end());
                }
                else
                {
                    // Place the next level of outer polygon nodes on the
                    // queue for triangulation.
                    std::vector<Polygon> inners(numChildren);
                    for (int32_t c = 0; c < numChildren; ++c)
                    {
                        std::shared_ptr<PolygonTree> inner = outer->child[c];
                        inners[c] = inner->polygon;
                        int32_t numGrandChildren = static_cast<int32_t>(inner->child.size());
                        for (int32_t g = 0; g < numGrandChildren; ++g)
                        {
                            treeQueue.push(inner->child[g]);
                        }
                    }

                    // Combine the outer polygon and the inner polygons into a
                    // pseudosimple polygon using repeated calls to
                    // CombineSimple.
                    Polygon combined{};
                    CombineMultiple(outer->polygon, inners, combined);

                    // Triangulate the pseudosimple polygon using ear clipping.
                    std::vector<std::array<int32_t, 3>> combinedTriangles{};
                    mVertexList.DoEarClipping(combined, mComputePoints, mQuery, combinedTriangles);
                    mTriangles.insert(mTriangles.end(), combinedTriangles.begin(), combinedTriangles.end());
                }
            }
        }

    private:
        // The input vertex pool.
        int32_t const mNumPoints;
        Vector2<InputType> const* mPoints;

        // The output triangulation.
        std::vector<std::array<int32_t, 3>> mTriangles;

    private:
        // Support for rational arithmetic. The converters transform points
        // with InputType components to points with ComputeType components.
        // If you want to be certain of a correct result, choose ComputeType
        // to be BSRational.
        void ConvertPoints(Polygon const& polygon)
        {
            for (auto const& index : polygon)
            {
                if (mConverted[index] == 0)
                {
                    mConverted[index] = 1;
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                    }
                }
            }
        }

        void ConvertPoints(Polygon const& outer, Polygon const& inner)
        {
            for (auto index : outer)
            {
                if (mConverted[index] == 0)
                {
                    mConverted[index] = 1;
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                    }
                }
            }

            for (auto index : inner)
            {
                if (mConverted[index] == 0)
                {
                    mConverted[index] = 1;
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                    }
                }
            }
        }

        void ConvertPoints(Polygon const& outer, std::vector<Polygon> const& inners)
        {
            for (auto index : outer)
            {
                if (mConverted[index] == 0)
                {
                    mConverted[index] = 1;
                    for (int32_t j = 0; j < 2; ++j)
                    {
                        mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                    }
                }
            }

            for (auto const& inner : inners)
            {
                for (auto index : inner)
                {
                    if (mConverted[index] == 0)
                    {
                        mConverted[index] = 1;
                        for (int32_t j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                        }
                    }
                }
            }
        }

        void ConvertPoints(std::shared_ptr<PolygonTree> const& tree)
        {
            std::queue<std::shared_ptr<PolygonTree>> treeQueue{};
            treeQueue.push(tree);
            while (!treeQueue.empty())
            {
                // The 'root' is an outer polygon.
                std::shared_ptr<PolygonTree> outer = treeQueue.front();
                treeQueue.pop();

                for (auto index : outer->polygon)
                {
                    if (mConverted[index] == 0)
                    {
                        mConverted[index] = 1;
                        for (int32_t j = 0; j < 2; ++j)
                        {
                            mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                        }
                    }
                }

                // The grandchildren of the outer polygon are also outer
                // polygons. Insert them into the queue for processing.
                int32_t numChildren = static_cast<int32_t>(outer->child.size());
                for (int32_t c = 0; c < numChildren; ++c)
                {
                    // The 'child' is an inner polygon.
                    std::shared_ptr<PolygonTree> inner = outer->child[c];
                    for (auto index : inner->polygon)
                    {
                        if (mConverted[index] == 0)
                        {
                            mConverted[index] = 1;
                            for (int32_t j = 0; j < 2; ++j)
                            {
                                mComputePoints[index][j] = static_cast<ComputeType>(mPoints[index][j]);
                            }
                        }
                    }

                    int32_t numGrandChildren = static_cast<int32_t>(inner->child.size());
                    for (int32_t g = 0; g < numGrandChildren; ++g)
                    {
                        treeQueue.push(inner->child[g]);
                    }
                }
            }
        }

        // The array of points used for geometric queries. If you want to be
        // certain of a correct result, choose ComputeType to be BSRational.
        // The InputType points are converted to ComputeType points on demand.
        // The mConverted array keeps track of which input points have been
        // converted.
        std::vector<Vector2<ComputeType>> mComputePoints;
        std::vector<uint32_t> mConverted;

        // The object used for ToLine and ToTriangle queries.
        PrimalQuery2<ComputeType> mQuery;

    private:
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();
        using XMaxInfo = std::pair<InputType, size_t>;

        // The number pair.first is the maximum x-value of the polygon
        // vertices. The number pair.second is the index of the vertex that
        // generates a maximum x-value. It is not a problem if the maximum is
        // attained by more than one vertex. It is sufficient to use mPoints
        // directly because the InputType comparisons are exact.
        XMaxInfo GetXMaxInfo(Polygon const& polygon) const
        {
            InputType x = mPoints[polygon[0]][0];
            XMaxInfo xmaxInfo = std::make_pair(x, 0);
            for (size_t i = 1; i < polygon.size(); ++i)
            {
                x = mPoints[polygon[i]][0];
                if (x > xmaxInfo.first)
                {
                    xmaxInfo = std::make_pair(x, i);
                }
            }
            return xmaxInfo;
        }

        // Find the edge whose intersection Intr with the ray M + t * (1,0)
        // minimizes the ray parameter t >= 0. The inputs v0min, v1min and
        // endMin must be initialized to 'invalid'.
        Vector2<ComputeType> ComputeNearestOuterPolygonIntersection(
            Vector2<ComputeType> const& M, Polygon const& outer,
            size_t& v0min, size_t& v1min, size_t& endMin) const
        {
            auto const cmax = static_cast<ComputeType>(std::numeric_limits<InputType>::max());
            auto const zero = static_cast<ComputeType>(0);

            Vector2<ComputeType> intr{ cmax, M[1] };
            ComputeType s = cmax, t = cmax;
            size_t const numOuter = outer.size();
            for (size_t i0 = numOuter - 1, i1 = 0; i1 < numOuter; i0 = i1++)
            {
                // Consider only edges for which the first vertex is below (or
                // on) the ray and the second vertex is above (or on) the ray.
                Vector2<ComputeType> diff0 = mComputePoints[outer[i0]] - M;
                if (diff0[1] > zero)
                {
                    continue;
                }
                Vector2<ComputeType> diff1 = mComputePoints[outer[i1]] - M;
                if (diff1[1] < zero)
                {
                    continue;
                }

                // At this time, diff0.y <= 0 and diff1.y >= 0.
                size_t currentEndMin = invalid;
                if (diff0[1] < zero)
                {
                    if (diff1[1] > zero)
                    {
                        // The intersection of the edge and ray occurs at an
                        // interior edge point.
                        s = diff0[1] / (diff0[1] - diff1[1]);
                        t = diff0[0] + s * (diff1[0] - diff0[0]);
                    }
                    else  // diff1.y == 0
                    {
                        // The vertex Outer[i1] is the intersection of the
                        // edge and the ray.
                        t = diff1[0];
                        currentEndMin = i1;
                    }
                }
                else  // diff0.y == 0
                {
                    if (diff1[1] > zero)
                    {
                        // The vertex Outer[i0] is the intersection of the
                        // edge and the ray;
                        t = diff0[0];
                        currentEndMin = i0;
                    }
                    else  // diff1.y == 0
                    {
                        if (diff0[0] < diff1[0])
                        {
                            t = diff0[0];
                            currentEndMin = i0;
                        }
                        else
                        {
                            t = diff1[0];
                            currentEndMin = i1;
                        }
                    }
                }

                if (zero <= t && t < intr[0])
                {
                    intr[0] = t;
                    v0min = i0;
                    v1min = i1;
                    if (currentEndMin == invalid)
                    {
                        // The current closest point is an edge-interior
                        // point.
                        endMin = invalid;
                    }
                    else
                    {
                        // The current closest point is a vertex.
                        endMin = currentEndMin;
                    }
                }
                else if (t == intr[0])
                {
                    // The current closest point is a vertex shared by
                    // multiple edges; thus, endMin and currentMin refer to
                    // the same point.
                    LogAssert(
                        endMin != invalid && currentEndMin != invalid,
                        "Unexpected condition.");

                    // We need to select the edge closest to M. The previous
                    // closest edge is <outer[v0min],outer[v1min]>. The
                    // current candidate is <outer[i0],outer[i1]>.
                    Vector2<ComputeType> shared = mComputePoints[outer[i1]];

                    // For the previous closest edge, endMin refers to a
                    // vertex of the edge. Get the index of the other vertex.
                    size_t other = (endMin == v0min ? v1min : v0min);

                    // The new edge is closer if the other vertex of the old
                    // edge is left-of the new edge.
                    diff0 = mComputePoints[outer[i0]] - shared;
                    diff1 = mComputePoints[outer[other]] - shared;
                    ComputeType dotperp = DotPerp(diff0, diff1);
                    if (dotperp > zero)
                    {
                        // The new edge is closer to M.
                        v0min = i0;
                        v1min = i1;
                        endMin = currentEndMin;
                    }
                }
            }

            // The intersection intr[0] stored only the t-value of the ray.
            // The actual point is (mx,my)+t*(1,0), so intr[0] must be
            // adjusted.
            intr[0] += M[0];
            return intr;
        }

        size_t LocateOuterVisibleVertex(Vector2<ComputeType> const& M,
            Vector2<ComputeType> const& I, Polygon const& outer,
            size_t v0min, size_t v1min, size_t endMin) const
        {
            // The point mPoints[outer[oVisibleIndex]] maximizes the cosine
            // of the angle between <M,I> and <M,Q> where Q is P or a reflex
            // vertex contained in triangle <M,I,P>.
            size_t oVisibleIndex = endMin;
            if (endMin == invalid)
            {
                // If you reach this assert, there is a good chance that you
                // have two inner polygons that share a vertex or an edge.
                LogAssert(
                    v0min >= 0 && v1min >= 0,
                    "Is this an invalid nested polygon?");

                // Select mPoints[outer[v0min]] or mPoints[outer[v1min]] that
                // has an x-value larger than M.x, call this vertex P. The
                // triangle <M,I,P> must contain an outer-polygon vertex that
                // is visible to M, which is possibly P itself.
                std::array<Vector2<ComputeType>, 3> triangle{};
                size_t pIndex{};
                if (mComputePoints[outer[v0min]][0] > mComputePoints[outer[v1min]][0])
                {
                    auto const& P = mComputePoints[outer[v0min]];
                    triangle[0] = P;
                    triangle[1] = I;
                    triangle[2] = M;
                    pIndex = v0min;
                }
                else
                {
                    auto const& P = mComputePoints[outer[v1min]];
                    triangle[0] = P;
                    triangle[1] = M;
                    triangle[2] = I;
                    pIndex = v1min;
                }

                // If any outer-polygon vertices other than P are inside the
                // triangle <M,I,P>, then at least one of these vertices must
                // be a reflex vertex. It is sufficient to locate the reflex
                // vertex R (if any) in <M,I,P> that minimizes the angle
                // between R-M and (1,0).
                Vector2<ComputeType> diff = triangle[0] - M;
                ComputeType maxSqrLen = Dot(diff, diff);
                ComputeType maxCos = diff[0] * diff[0] / maxSqrLen;
                PrimalQuery2<ComputeType> localQuery(3, triangle.data());
                size_t const numOuter = outer.size();
                oVisibleIndex = pIndex;
                for (size_t i = 0; i < numOuter; ++i)
                {
                    if (i == pIndex)
                    {
                        continue;
                    }

                    int32_t curr = static_cast<int32_t>(outer[i]);
                    int32_t prev = static_cast<int32_t>(outer[(i + numOuter - 1) % numOuter]);
                    int32_t next = static_cast<int32_t>(outer[(i + 1) % numOuter]);
                    if (mQuery.ToLine(curr, prev, next) <= 0
                        && localQuery.ToTriangle(mComputePoints[curr], 0, 1, 2) <= 0)
                    {
                        // The vertex is reflex and inside the <M,I,P>
                        // triangle.
                        diff = mComputePoints[curr] - M;
                        ComputeType sqrLen = Dot(diff, diff);
                        ComputeType cs = diff[0] * diff[0] / sqrLen;
                        if (cs > maxCos)
                        {
                            // The reflex vertex forms a smaller angle with
                            // the positive x-axis, so it becomes the new
                            // visible candidate.
                            maxSqrLen = sqrLen;
                            maxCos = cs;
                            oVisibleIndex = i;
                        }
                        else if (cs == maxCos && sqrLen < maxSqrLen)
                        {
                            // The reflex vertex has angle equal to the
                            // current minimum but the length is smaller, so
                            // it becomes the new visible candidate.
                            maxSqrLen = sqrLen;
                            oVisibleIndex = i;
                        }
                    }
                }
            }

            return oVisibleIndex;
        }

        void CombineSingle(Polygon const& outer, Polygon const& inner,
            Polygon& combined)
        {
            // Get the index into inner[] for the inner-polygon vertex M of
            // maximum x-value.
            size_t iVisibleIndex = GetXMaxInfo(inner).second;

            // Get the inner-polygon vertex M of maximum x-value.
            size_t iVertexIndex = inner[iVisibleIndex];
            Vector2<ComputeType> const& M = mComputePoints[iVertexIndex];

            // Compute the closest outer-polygon point I along the ray
            // M + t *(1,0) with t > 0 so that M and I are mutually visible.
            size_t v0min = invalid, v1min = invalid, endMin = invalid;
            Vector2<ComputeType> I = ComputeNearestOuterPolygonIntersection(
                M, outer, v0min, v1min, endMin);

            // Locate Q = mPoints[outer[oVisibleIndex]] so that M and Q are
            // mutually visible.
            size_t oVisibleIndex = LocateOuterVisibleVertex(
                M, I, outer, v0min, v1min, endMin);

            InsertBridge(outer, inner, oVisibleIndex, iVisibleIndex, combined);
        }

        void CombineMultiple(Polygon const& outer, std::vector<Polygon> const& inners,
            Polygon& combined)
        {
            // Sort the inner polygons based on maximum x-values.
            using PairType = std::pair<ComputeType, size_t>;
            size_t numInners = inners.size();
            std::vector<PairType> pairs(numInners);
            for (size_t p = 0; p < numInners; ++p)
            {
                size_t numIndices = inners[p].size();
                int32_t const* indices = inners[p].data();

                InputType xmax = mPoints[indices[0]][0];
                for (size_t i = 1; i < numIndices; ++i)
                {
                    InputType x = mPoints[indices[i]][0];
                    if (x > xmax)
                    {
                        xmax = x;
                    }
                }

                pairs[p].first = xmax;
                pairs[p].second = p;
            }
            std::sort(pairs.begin(), pairs.end(), std::greater<PairType>());

            Polygon currentOuter = outer;
            for (auto const& pair : pairs)
            {
                Polygon const& inner = inners[pair.second];
                Polygon currentCombined{};
                CombineSingle(currentOuter, inner, currentCombined);
                currentOuter = std::move(currentCombined);
            }
            combined = std::move(currentOuter);
        }

        // The mutually visible vertices are VI = mPoints[inner[iVisibleIndex]]
        // and VO = mPoints[outer[oVisibleIndex]]. Two coincident edges with
        // these endpoints are inserted to connect the outer and inner polygons
        // into a pseudosimple polygon.
        void InsertBridge(Polygon const& outer, Polygon const& inner,
            size_t oVisibleIndex, size_t iVisibleIndex, Polygon& combined)
        {
            size_t const numOuter = outer.size();
            size_t const numInner = inner.size();
            combined.resize(numOuter + numInner + 2);

            // Traverse the outer polygon until the outer polygon bridge.
            // point is visited.
            size_t cIndex = 0;
            for (size_t i = 0; i <= oVisibleIndex; ++i, ++cIndex)
            {
                combined[cIndex] = outer[i];
            }

            // Cross the bridge from the outer polygon to the inner polygon.
            // Traverse the inner polygon until the predecessor of the inner
            // polygon bridge point is visited.
            for (size_t i = 0; i < numInner; ++i, ++cIndex)
            {
                size_t j = (iVisibleIndex + i) % numInner;
                combined[cIndex] = inner[j];
            }

            // Inner polygon bridge point.
            combined[cIndex++] = inner[iVisibleIndex];

            // Outer polygon bridge point.
            combined[cIndex++] = outer[oVisibleIndex];

            for (size_t i = oVisibleIndex + 1; i < numOuter; ++i, ++cIndex)
            {
                combined[cIndex] = outer[i];
            }
        }

    private:
        // A doubly linked list for storing specially tagged vertices (convex,
        // reflex, ear). The vertex list is used for ear clipping.
        struct Vertex
        {
            Vertex()
                :
                index(-1),
                vPrev(-1),
                vNext(-1),
                sPrev(-1),
                sNext(-1),
                ePrev(-1),
                eNext(-1),
                isConvex(false),
                isEar(false)
            {
            }

            int32_t index;          // index of vertex in mPoints array
            int32_t vPrev, vNext;   // vertex links for polygon
            int32_t sPrev, sNext;   // convex/reflex vertex links (disjoint lists)
            int32_t ePrev, eNext;   // ear links
            bool isConvex, isEar;
        };

        class VertexList
        {
        public:
            VertexList()
                :
                mVertices{},
                mCFirst(-1),
                mCLast(-1),
                mRFirst(-1),
                mRLast(-1),
                mEFirst(-1),
                mELast(-1)
            {
            }

            void DoEarClipping(
                Polygon const& polygon,
                std::vector<Vector2<ComputeType>> const& computePoints,
                PrimalQuery2<ComputeType> const& query,
                std::vector<std::array<int32_t, 3>>& triangles)
            {
                triangles.clear();

                // Initialize the vertex list for the incoming polygon.
                // The lists must be cleared in case a single VertexList
                // object is used two or more times in triangulation
                // queries. This is the case for triangulating a polygon
                // tree. It is also the case if you use a single
                // TriangulateEC object for multiple triangulation queries.
                mVertices.resize(polygon.size());
                mCFirst = -1;
                mCLast = -1;
                mRFirst = -1;
                mRLast = -1;
                mEFirst = -1;
                mELast = -1;

                // Create a circular list of the polygon vertices for dynamic
                // removal of vertices.
                int32_t numVertices = static_cast<int32_t>(polygon.size());
                int32_t const* indices = polygon.data();
                for (int32_t i = 0, ip1 = 1; ip1 <= numVertices; i = ip1++)
                {
                    Vertex& vertex = mVertices[i];
                    vertex.index = indices[i];
                    vertex.vPrev = (i > 0 ? i - 1 : numVertices - 1);
                    vertex.vNext = (ip1 < numVertices ? ip1 : 0);

                    // These members must be cleared in case a single
                    // VertexList object is used two or more times in
                    // triangulation queries. This is the case for
                    // triangulating a polygon tree. It is also the case if
                    // you use a single TriangulateEC object for multiple
                    // triangulation queries.
                    vertex.sPrev = -1;
                    vertex.sNext = -1;
                    vertex.ePrev = -1;
                    vertex.eNext = -1;
                    vertex.isConvex = false;
                    vertex.isEar = false;
                }

                // Create a circular list of the polygon vertices for dynamic
                // removal of vertices. Keep track of two linear sublists, one
                // for the convex vertices and one for the reflex vertices.
                // This is an O(N) process where N is the number of polygon
                // vertices.
                for (int32_t i = 0; i < numVertices; ++i)
                {
                    if (IsConvex(i, query))
                    {
                        InsertAfterC(i);
                    }
                    else
                    {
                        InsertAfterR(i);
                    }
                }

                // If the polygon is convex, create a triangle fan.
                if (mRFirst == -1)
                {
                    for (int32_t i = 1, ip1 = 2; ip1 < numVertices; i = ip1++)
                    {
                        triangles.push_back({ polygon[0], polygon[i], polygon[ip1] });
                    }
                    return;
                }

                // Identify the ears and build a circular list of them. Let
                // V0, V1, and V2 be consecutive vertices forming triangle T.
                // The vertex V1 is an ear if no other vertices of the polygon
                // lie inside T. Although it is enough to show that V1 is not
                // an ear by finding at least one other vertex inside T, it is
                // sufficient to search only the reflex vertices. This is an
                // O(C*R) process, where C is the number of convex vertices
                // and R is the number of reflex vertices with N = C+R. The
                // order is O(N^2), for example when C = R = N/2.
                for (int32_t i = mCFirst; i != -1; i = V(i).sNext)
                {
                    if (IsEar(i, computePoints, query))
                    {
                        InsertEndE(i);
                    }
                }
                V(mEFirst).ePrev = mELast;
                V(mELast).eNext = mEFirst;

                // Remove the ears, one at a time.
                bool bRemoveAnEar = true;
                while (bRemoveAnEar)
                {
                    // Add the triangle with the ear to the output list of
                    // triangles.
                    int32_t iVPrev = V(mEFirst).vPrev;
                    int32_t iVNext = V(mEFirst).vNext;
                    triangles.push_back({ V(iVPrev).index, V(mEFirst).index, V(iVNext).index });

                    // Remove the vertex corresponding to the ear.
                    RemoveV(mEFirst);
                    if (--numVertices == 3)
                    {
                        // Only one triangle remains, just remove the ear and
                        // copy it.
                        mEFirst = RemoveE(mEFirst);
                        iVPrev = V(mEFirst).vPrev;
                        iVNext = V(mEFirst).vNext;
                        triangles.push_back({ V(iVPrev).index, V(mEFirst).index, V(iVNext).index });
                        bRemoveAnEar = false;
                        continue;
                    }

                    // Removal of the ear can cause an adjacent vertex to
                    // become an ear or to stop being an ear.
                    Vertex& vPrev = V(iVPrev);
                    if (vPrev.isEar)
                    {
                        if (!IsEar(iVPrev, computePoints, query))
                        {
                            RemoveE(iVPrev);
                        }
                    }
                    else
                    {
                        bool wasReflex = !vPrev.isConvex;
                        if (IsConvex(iVPrev, query))
                        {
                            if (wasReflex)
                            {
                                RemoveR(iVPrev);
                            }

                            if (IsEar(iVPrev, computePoints, query))
                            {
                                InsertBeforeE(iVPrev);
                            }
                        }
                    }

                    Vertex& vNext = V(iVNext);
                    if (vNext.isEar)
                    {
                        if (!IsEar(iVNext, computePoints, query))
                        {
                            RemoveE(iVNext);
                        }
                    }
                    else
                    {
                        bool wasReflex = !vNext.isConvex;
                        if (IsConvex(iVNext, query))
                        {
                            if (wasReflex)
                            {
                                RemoveR(iVNext);
                            }

                            if (IsEar(iVNext, computePoints, query))
                            {
                                InsertAfterE(iVNext);
                            }
                        }
                    }

                    // Remove the ear.
                    mEFirst = RemoveE(mEFirst);
                }
            }

        private:
            Vertex& V(int32_t i)
            {
                // If the assertion is triggered, do you have a coincident
                // vertex-edge or edge-edge pair? These violate the assumptions
                // for the algorithm.
                LogAssert(
                    0 <= i && i < static_cast<int32_t>(mVertices.size()),
                    "Index out of range..");
                return mVertices[i];
            }

            bool IsConvex(int32_t i, PrimalQuery2<ComputeType> const& query)
            {
                Vertex& vertex = V(i);
                int32_t curr = vertex.index;
                int32_t prev = V(vertex.vPrev).index;
                int32_t next = V(vertex.vNext).index;
                vertex.isConvex = (query.ToLine(curr, prev, next) > 0);
                return vertex.isConvex;
            }

            bool IsEar(
                int32_t i,
                std::vector<Vector2<ComputeType>> const& computePoints,
                PrimalQuery2<ComputeType> const& query)
            {
                Vertex& vertex = V(i);

                if (mRFirst == -1)
                {
                    // The remaining polygon is convex.
                    vertex.isEar = true;
                    return true;
                }

                // Search the reflex vertices and test if any are in the triangle
                // <V[prev],V[curr],V[next]>.
                int32_t prev = V(vertex.vPrev).index;
                int32_t curr = vertex.index;
                int32_t next = V(vertex.vNext).index;
                vertex.isEar = true;
                for (int32_t j = mRFirst; j != -1; j = V(j).sNext)
                {
                    // Check if the test vertex is already one of the triangle
                    // vertices.
                    if (j == vertex.vPrev || j == i || j == vertex.vNext)
                    {
                        continue;
                    }

                    // V[j] has been ruled out as one of the original vertices of
                    // the triangle <V[prev],V[curr],V[next]>. When triangulating
                    // polygons with holes, V[j] might be a duplicated vertex, in
                    // which case it does not affect the earness of V[curr].
                    int32_t testIndex = V(j).index;
                    Vector2<ComputeType> const& testPoint = computePoints[testIndex];
                    if (testPoint == computePoints[prev] ||
                        testPoint == computePoints[curr] ||
                        testPoint == computePoints[next])
                    {
                        continue;
                    }

                    // Test if the vertex is inside or on the triangle. When it
                    // is, it causes V[curr] not to be an ear.
                    if (query.ToTriangle(testIndex, prev, curr, next) <= 0)
                    {
                        vertex.isEar = false;
                        break;
                    }
                }

                return vertex.isEar;
            }

            // Insert a convex vertex.
            void InsertAfterC(int32_t i)
            {
                if (mCFirst == -1)
                {
                    // Insert the first convex vertex.
                    mCFirst = i;
                }
                else
                {
                    V(mCLast).sNext = i;
                    V(i).sPrev = mCLast;
                }
                mCLast = i;
            }

            // Insert a reflex vertex.
            void InsertAfterR(int32_t i)
            {
                if (mRFirst == -1)
                {
                    // Insert the first reflex vertex.
                    mRFirst = i;
                }
                else
                {
                    V(mRLast).sNext = i;
                    V(i).sPrev = mRLast;
                }
                mRLast = i;
            }

            // Insert an ear at the end of the list.
            void InsertEndE(int32_t i)
            {
                if (mEFirst == -1)
                {
                    // Insert the first ear.
                    mEFirst = i;
                    mELast = i;
                }
                V(mELast).eNext = i;
                V(i).ePrev = mELast;
                mELast = i;
            }

            // Insert an ear after mEFirst.
            void InsertAfterE(int32_t i)
            {
                Vertex& first = V(mEFirst);
                int32_t currENext = first.eNext;
                Vertex& vertex = V(i);
                vertex.ePrev = mEFirst;
                vertex.eNext = currENext;
                first.eNext = i;
                V(currENext).ePrev = i;
            }

            // Insert an ear before mEFirst.
            void InsertBeforeE(int32_t i)
            {
                Vertex& first = V(mEFirst);
                int32_t currEPrev = first.ePrev;
                Vertex& vertex = V(i);
                vertex.ePrev = currEPrev;
                vertex.eNext = mEFirst;
                first.ePrev = i;
                V(currEPrev).eNext = i;
            }

            // Remove a vertex.
            void RemoveV(int32_t i)
            {
                int32_t currVPrev = V(i).vPrev;
                int32_t currVNext = V(i).vNext;
                V(currVPrev).vNext = currVNext;
                V(currVNext).vPrev = currVPrev;
            }

            // Remove an ear.
            int32_t RemoveE(int32_t i)
            {
                int32_t currEPrev = V(i).ePrev;
                int32_t currENext = V(i).eNext;
                V(currEPrev).eNext = currENext;
                V(currENext).ePrev = currEPrev;
                return currENext;
            }

            // Remove a reflex vertex.
            void RemoveR(int32_t i)
            {
                LogAssert(
                    mRFirst != -1 && mRLast != -1,
                    "Reflex vertices must exist.");

                if (i == mRFirst)
                {
                    mRFirst = V(i).sNext;
                    if (mRFirst != -1)
                    {
                        V(mRFirst).sPrev = -1;
                    }
                    V(i).sNext = -1;
                }
                else if (i == mRLast)
                {
                    mRLast = V(i).sPrev;
                    if (mRLast != -1)
                    {
                        V(mRLast).sNext = -1;
                    }
                    V(i).sPrev = -1;
                }
                else
                {
                    int32_t currSPrev = V(i).sPrev;
                    int32_t currSNext = V(i).sNext;
                    V(currSPrev).sNext = currSNext;
                    V(currSNext).sPrev = currSPrev;
                    V(i).sNext = -1;
                    V(i).sPrev = -1;
                }
            }

            // The doubly linked list.
            std::vector<Vertex> mVertices;
            int32_t mCFirst, mCLast;  // linear list of convex vertices
            int32_t mRFirst, mRLast;  // linear list of reflex vertices
            int32_t mEFirst, mELast;  // cyclical list of ears
        };

        VertexList mVertexList;
    };
}
