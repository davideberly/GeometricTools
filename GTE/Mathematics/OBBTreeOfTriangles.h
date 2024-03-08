// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.08.08

#pragma once

// Read the comments in OBBTree.h regarding tree construction.

#include <Mathematics/IntrLine3OrientedBox3.h>
#include <Mathematics/IntrRay3OrientedBox3.h>
#include <Mathematics/IntrSegment3OrientedBox3.h>
#include <Mathematics/IntrLine3Triangle3.h>
#include <Mathematics/IntrRay3Triangle3.h>
#include <Mathematics/IntrSegment3Triangle3.h>
#include <Mathematics/OBBTree.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>

namespace gte
{
    template <typename T>
    class OBBTreeOfTriangles : public OBBTree<T>
    {
    public:
        OBBTreeOfTriangles()
            :
            OBBTree<T>{},
            mVertices{},
            mTriangles{},
            mBoxQueries{ IntersectLineBox, IntersectRayBox, IntersectSegmentBox },
            mTriangleQueries{ IntersectLineTriangle, IntersectRayTriangle, IntersectSegmentTriangle }
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& vertices,
            std::vector<std::array<size_t, 3>> const& triangles,
            size_t height = std::numeric_limits<size_t>::max())
        {
            LogAssert(
                vertices.size() >= 3 && triangles.size() > 0,
                "Invalid input.");

            mVertices = vertices;
            mTriangles = triangles;

            // Compute the triangle centroids.
            std::vector<Vector3<T>> centroids(mTriangles.size());
            T const three = static_cast<T>(3);
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                centroids[t] = (mVertices[tri[0]] + mVertices[tri[1]] + mVertices[tri[2]]) / three;
            }

            // Create the OBB tree for centroids.
            OBBTree<T>::Create(centroids, height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<std::array<size_t, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

    private:
        // Let C be the box center and let U0, U1 and U2 be the box axes.
        // Each input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.
        // The following code computes min(y0), max(y0), min(y1), max(y1),
        // min(y2), and max(y2). The box center is then adjusted to be
        //   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1
        //        + 0.5*(min(y2)+max(y2))*U2
        virtual void ComputeInteriorBox(size_t i0, size_t i1, OrientedBox3<T>& box) override
        {
            OBBTree<T>::ComputeInteriorBox(i0, i1, box);

            Vector3<T> pmin = Vector3<T>::Zero(), pmax = pmin;
            for (size_t i = i0; i <= i1; ++i)
            {
                auto const& tri = mTriangles[this->mPartition[i]];
                for (size_t k = 0; k < 3; ++k)
                {
                    Vector3<T> diff = mVertices[tri[k]] - box.center;
                    for (int32_t j = 0; j < 3; ++j)
                    {
                        T dot = Dot(diff, box.axis[j]);
                        if (dot < pmin[j])
                        {
                            pmin[j] = dot;
                        }
                        else if (dot > pmax[j])
                        {
                            pmax[j] = dot;
                        }
                    }
                }
            }

            T const half = static_cast<T>(0.5);
            for (int32_t j = 0; j < 3; ++j)
            {
                box.center += (half * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = half * (pmax[j] - pmin[j]);
            }
        }

        virtual void ComputeLeafBox(size_t i, OrientedBox3<T>& box) override
        {
            // Create a degenerate box whose center is the midpoint of the
            // triangle primitive, whose axis[0] is the direction of a
            // triangle edge, whose axis[2] is a triangle normal, and whose
            // axis[1] is Cross(axis[2], axis[0]). The extent[0] and extent[1]
            // are chosen so that the box contains the triangle. The extent[2]
            // is zero.
            auto const& tri = mTriangles[this->mPartition[i]];
            Vector3<T> edge10 = mVertices[tri[1]] - mVertices[tri[0]];
            Vector3<T> edge20 = mVertices[tri[2]] - mVertices[tri[0]];
            Normalize(edge10);
            Normalize(edge20);
            Vector3<T> normal = UnitCross(edge10, edge20);

            box.center = this->mCentroids[this->mPartition[i]];
            box.axis[0] = edge10;
            box.axis[1] = Cross(normal, edge10);
            box.axis[2] = normal;

            Vector3<T> V0mC = mVertices[tri[0]] - box.center;
            Vector3<T> V1mC = mVertices[tri[1]] - box.center;
            Vector3<T> V2mC = mVertices[tri[2]] - box.center;
            T ax0 = std::fabs(Dot(box.axis[0], V0mC));
            T ax1 = std::fabs(Dot(box.axis[0], V1mC));
            T ax2 = std::fabs(Dot(box.axis[0], V2mC));
            T ay0 = std::fabs(Dot(box.axis[1], V0mC));
            T ay1 = std::fabs(Dot(box.axis[1], V1mC));
            T ay2 = std::fabs(Dot(box.axis[1], V2mC));
            box.extent[0] = std::max(ax0, std::max(ax1, ax2));
            box.extent[1] = std::max(ay0, std::max(ay1, ay2));
            box.extent[2] = static_cast<T>(0);
        }

        std::vector<Vector3<T>> mVertices;
        std::vector<std::array<size_t, 3>> mTriangles;

    public:
        // Generate a list of triangles intersected by a linear component
        // (line, ray or segment). The line is parameterized by P + t * Q,
        // where Q is a unit-length direction and t is any real number. The
        // ray is parameterized by P + t * Q, where Q is a unit-length
        // direction and t >= 0. The segment is parameterized by
        // (1-t) * P + t * Q = P + t * (Q - P), where P and Q are the
        // endpoints of the segment and 0 <= t <= 1.

        static uint32_t constexpr LINE_QUERY = 0;
        static uint32_t constexpr RAY_QUERY = 1;
        static uint32_t constexpr SEGMENT_QUERY = 2;

        struct Intersection
        {
            Intersection()
                :
                triangleIndex(std::numeric_limits<size_t>::max()),
                point(Vector3<T>::Zero()),
                parameter(static_cast<T>(0))
            {
            }

            Intersection(size_t inTriangleIndex, Vector3<T> const& inPoint, T const& inParameter)
                :
                triangleIndex(inTriangleIndex),
                point(inPoint),
                parameter(inParameter)
            {
            }

            bool operator<(Intersection const& other) const
            {
                return parameter < other.parameter;
            }

            size_t triangleIndex;
            Vector3<T> point;
            T parameter;
        };

        // The intersections are sorted by the parameter.
        void Execute(uint32_t queryType, Vector3<T> const& P, Vector3<T> const& Q,
            std::set<Intersection>& intersections)
        {
            size_t constexpr invalid = std::numeric_limits<size_t>::max();
            intersections.clear();

            std::vector<size_t> indexStack(2 * this->mHeight);
            size_t top = 0;
            indexStack[0] = 0;
            while (top != std::numeric_limits<size_t>::max())
            {
                size_t nodeIndex = indexStack[top--];
                auto const& node = this->mNodes[nodeIndex];

                // For tne balanced tree created by OBBTree<T>, an interior
                // node has two valid children and a leaf node has two invalid
                // children. This is true even if the height passed to
                // OBBTree<T>::Create is smaller than the actual height.
                if (node.leftChild != invalid && node.rightChild != invalid)
                {
                    // The node is interior.
                    if (mBoxQueries[queryType](P, Q, node.box))
                    {
                        // The linear component intersects the box. Continue
                        // the intersection search to child nodes if they
                        // exist.
                        indexStack[++top] = node.rightChild;
                        indexStack[++top] = node.leftChild;
                    }
                    else
                    {
                        // The linear component does not intersect the box.
                        // There are no triangles intersected in the subtree
                        // rooted at this node. Do not continue the
                        // intersection search to child nodes if they exist.
                    }
                }
                else // node.leftChild == invalid && node.rightChild == invalid
                {
                    for (size_t i = node.minIndex; i <= node.maxIndex; ++i)
                    {
                        size_t triangleIndex = this->mPartition[i];
                        auto const& tri = mTriangles[triangleIndex];
                        Triangle3<T> triangle(mVertices[tri[0]], mVertices[tri[1]], mVertices[tri[2]]);
                        auto triResult = mTriangleQueries[queryType](P, Q, triangle);
                        if (triResult.intersect)
                        {
                            intersections.insert(Intersection(triangleIndex,
                                triResult.point, triResult.parameter));
                        }
                    }
                }
            }
        }

    private:
        using BoxQuery = bool (*)(
            Vector3<T> const&, Vector3<T> const&, OrientedBox3<T> const&);

        static bool IntersectLineBox(Vector3<T> const& P, Vector3<T> const& Q,
            OrientedBox3<T> const& box)
        {
            TIQuery<T, Line3<T>, OrientedBox3<T>> query{};
            auto result = query(Line3<T>(P, Q), box);
            return result.intersect;
        }

        static bool IntersectRayBox(Vector3<T> const& P, Vector3<T> const& Q,
            OrientedBox3<T> const& box)
        {
            TIQuery<T, Ray3<T>, OrientedBox3<T>> query{};
            auto result = query(Ray3<T>(P, Q), box);
            return result.intersect;
        }

        static bool IntersectSegmentBox(Vector3<T> const& P, Vector3<T> const& Q,
            OrientedBox3<T> const& box)
        {
            TIQuery<T, Segment3<T>, OrientedBox3<T>> query{};
            auto result = query(Segment3<T>(P, Q), box);
            return result.intersect;
        }

        struct TriangleResult
        {
            bool intersect;
            Vector3<T> point;
            T parameter;
        };

        using TriangleQuery = TriangleResult (*)(
            Vector3<T> const&, Vector3<T> const&, Triangle3<T> const&);

        static TriangleResult IntersectLineTriangle(Vector3<T> const& P,
            Vector3<T> const& Q, Triangle3<T> const& triangle)
        {
            FIQuery<T, Line3<T>, Triangle3<T>> query{};
            auto result = query(Line3<T>(P, Q), triangle);

            TriangleResult triResult{};
            triResult.intersect = result.intersect;
            triResult.point = result.point;
            triResult.parameter = result.parameter;
            return triResult;
        }

        static TriangleResult IntersectRayTriangle(Vector3<T> const& P,
            Vector3<T> const& Q, Triangle3<T> const& triangle)
        {
            FIQuery<T, Ray3<T>, Triangle3<T>> query{};
            auto result = query(Ray3<T>(P, Q), triangle);

            TriangleResult triResult{};
            triResult.intersect = result.intersect;
            triResult.point = result.point;
            triResult.parameter = result.parameter;
            return triResult;
        }

        static TriangleResult IntersectSegmentTriangle(Vector3<T> const& P,
            Vector3<T> const& Q, Triangle3<T> const& triangle)
        {
            FIQuery<T, Segment3<T>, Triangle3<T>> query{};
            auto result = query(Segment3<T>(P, Q), triangle);

            // The segment is converted to centered form in the query. That
            // form is C + s * D, where C is the midpoint of the segment,
            // D is a unit-length vector and |s| <= e for segment extent
            // (half length) e. The t-parameter must be converted back to
            // (1-t)*P+t*Q where t in [0,1]. Thus, t = (s+e)/(2*e) which
            // is equivalent to s/Length(Q-P)+1/2.
            TriangleResult triResult{};
            triResult.intersect = result.intersect;
            triResult.point = result.point;
            triResult.parameter = result.parameter / Length(Q - P) + static_cast<T>(0.5);
            return triResult;
        }

        std::array<BoxQuery, 3> mBoxQueries;
        std::array<TriangleQuery, 3> mTriangleQueries;
    };
}
