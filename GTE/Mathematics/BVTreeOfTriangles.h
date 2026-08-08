// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2025 Geometric Tools LLC
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

// Read the comments in BVTree.h regarding tree construction. Although this
// class appears to be non-abstract, the BoundingVolume type has requirements
// for its interface. In this sense, BVTreeOfTriangles is abstract.

#include <Mathematics/BVTree.h>
#include <Mathematics/IntrLine3Triangle3.h>
#include <Mathematics/IntrRay3Triangle3.h>
#include <Mathematics/IntrSegment3Triangle3.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <utility>
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
            mLinearTriangleQuery{
                IntersectLineTriangle,
                IntersectRayTriangle,
                IntersectSegmentTriangle
            }
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<std::size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // vertices.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& vertices,
            std::vector<std::array<std::size_t, 3>> const& triangles,
            std::size_t height = std::numeric_limits<std::size_t>::max())
        {
            LogAssert(
                vertices.size() >= 3 && triangles.size() > 0,
                "Expecting at least 3 vertices and at least 1 triangle.");

            mVertices = vertices;
            mTriangles = triangles;

            // Compute the triangle centroids.
            std::vector<Vector3<T>> centroids(mTriangles.size());
            T const three = static_cast<T>(3);
            for (std::size_t t = 0; t < mTriangles.size(); ++t)
            {
                auto const& tri = mTriangles[t];
                centroids[t] = (mVertices[tri[0]] + mVertices[tri[1]] + mVertices[tri[2]]) / three;
            }

            // Create the bounding volume tree for centroids.
            BVTree<T, BoundingVolume>::Create(std::move(centroids), height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<std::array<std::size_t, 3>> const& GetTriangles() const
        {
            return mTriangles;
        }

        class Intersection
        {
        public:
            Intersection()
                :
                triangleIndex(std::numeric_limits<std::size_t>::max()),
                point{},
                parameter(static_cast<T>(0))
            {
            }

            Intersection(
                std::size_t inTriangleIndex,
                Vector3<T> const& inPoint,
                T const& inParameter)
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

            std::size_t triangleIndex;
            Vector3<T> point;
            T parameter;
        };

        // Compute intersections of the linear component and triangles. These
        // are sorted by the parameter of the linear component.
        void Execute(
            std::uint32_t queryType,
            Vector3<T> const& P,
            Vector3<T> const& Q,
            std::vector<std::size_t>& nodeIndices,
            std::set<Intersection>& intersections)
        {
            this->GetLeafIndices(queryType, P, Q, nodeIndices);

            LinearTriangleQuery linearTriangleQuery = mLinearTriangleQuery[queryType];
            Vector3<T> point{};
            T parameter{};
            intersections.clear();
            for (auto const& leafIndex : nodeIndices)
            {
                auto const& node = this->mNodes[leafIndex];
                for (std::size_t i = node.minIndex; i <= node.maxIndex; ++i)
                {
                    std::size_t triangleIndex = this->mPartition[i];
                    auto const& tri = mTriangles[triangleIndex];
                    Triangle3<T> triangle(mVertices[tri[0]], mVertices[tri[1]], mVertices[tri[2]]);
                    if (linearTriangleQuery(P, Q, triangle, point, parameter))
                    {
                        intersections.insert(Intersection(triangleIndex, point, parameter));
                    }
                }
            }
        }

    protected:
        using LinearTriangleQuery = bool (*)(Vector3<T> const&, Vector3<T> const&,
            Triangle3<T> const&, Vector3<T>&, T&);

        static bool IntersectLineTriangle(Vector3<T> const& P, Vector3<T> const& Q,
            Triangle3<T> const& triangle, Vector3<T>& point, T& parameter)
        {
            FIQuery<T, Line3<T>, Triangle3<T>> query{};
            auto output = query(Line3<T>(P, Q), triangle);
            point = output.point;
            parameter = output.parameter;
            return output.intersect;
        }

        static bool IntersectRayTriangle(Vector3<T> const& P, Vector3<T> const& Q,
            Triangle3<T> const& triangle, Vector3<T>& point, T& parameter)
        {
            FIQuery<T, Ray3<T>, Triangle3<T>> query{};
            auto output = query(Ray3<T>(P, Q), triangle);
            point = output.point;
            parameter = output.parameter;
            return output.intersect;
        }

        static bool IntersectSegmentTriangle(Vector3<T> const& P, Vector3<T> const& Q,
            Triangle3<T> const& triangle, Vector3<T>& point, T& parameter)
        {
            FIQuery<T, Segment3<T>, Triangle3<T>> query{};
            auto output = query(Segment3<T>(P, Q), triangle);
            point = output.point;
            parameter = output.parameter;
            return output.intersect;
        }

        std::vector<Vector3<T>> mVertices;
        std::vector<std::array<std::size_t, 3>> mTriangles;
        std::array<LinearTriangleQuery, 3> mLinearTriangleQuery;

    private:
        friend class UnitTestBVTreeOfTriangles;
    };
}
