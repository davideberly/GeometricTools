// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 7.1.2024.01.20

#pragma once

// Read the comments in BVTree.h regarding tree construction.

#include <Mathematics/BVTree.h>
#include <Mathematics/IntrLine3Triangle3.h>
#include <Mathematics/IntrRay3Triangle3.h>
#include <Mathematics/IntrSegment3Triangle3.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>

namespace gte
{
    template <typename T, typename BoundingVolume>
    class BVTreeOfTriangles : public BVTree<T, BoundingVolume>
    {
    public:
        BVTreeOfTriangles()
            :
            BVTree<T, BoundingVolume>{},
            mVertices{},
            mTriangles{},
            mBoundingVolumeQueries
            {
                BoundingVolume::IntersectLine,
                BoundingVolume::IntersectRay,
                BoundingVolume::IntersectSegment
            },
            mTriangleQueries
            {
                IntersectLineTriangle,
                IntersectRayTriangle,
                IntersectSegmentTriangle
            }
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
            this->mCentroids.resize(mTriangles.size());
            T const three = static_cast<T>(3);
            for (size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                this->mCentroids[t] =
                    (mVertices[tri[0]] + mVertices[tri[1]] + mVertices[tri[2]]) / three;
            }

            // Create the bounding volume tree for centroids.
            BVTree<T, BoundingVolume>::Create(height);
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
                    if (mBoundingVolumeQueries[queryType](P, Q, node.boundingVolume))
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

    protected:
        std::vector<Vector3<T>> mVertices;
        std::vector<std::array<size_t, 3>> mTriangles;

    private:
        // Function signature for {line,ray,segment}-boundingVolume
        // test-intersection queries.
        using BoundingVolumeQuery = bool (*)(
            Vector3<T> const&, Vector3<T> const&, BoundingVolume const&);

        // Support for {line,ray,segment}-triangle find-intersection queries.
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

        std::array<BoundingVolumeQuery, 3> mBoundingVolumeQueries;
        std::array<TriangleQuery, 3> mTriangleQueries;
    };
}
