// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

// Read the comments in BVTree.h regarding tree construction. Although this
// class appears to be non-abstract, the BoundingVolume type has requirements
// for its interface. In this sense, BVTreeOfSegments is abstract.

#include <Mathematics/BVTree.h>
#include <array>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T, typename BoundingVolume>
    class BVTreeOfSegments : public BVTree<T, BoundingVolume>
    {
    public:
        BVTreeOfSegments()
            :
            BVTree<T, BoundingVolume>{},
            mVertices{},
            mSegments{}
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<std::size_t>::max(),
        // the entire tree is built and the actual height is computed from
        // vertices.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& vertices,
            std::vector<std::array<std::size_t, 2>> const& segments,
            std::size_t height = std::numeric_limits<std::size_t>::max())
        {
            LogAssert(
                vertices.size() > 0,
                "Expecting vertices to create a bounding volume tree.");

            mVertices = vertices;
            mSegments = segments;

            // Compute the segment centroids.
            std::vector<Vector3<T>> centroids(mSegments.size());
            T const half = static_cast<T>(0.5);
            for (std::size_t i = 0; i < mSegments.size(); ++i)
            {
                auto const& seg = mSegments[i];
                centroids[i] = half * (mVertices[seg[0]] + mVertices[seg[1]]);
            }

            // Create the bounding volume tree for centroids.
            BVTree<T, BoundingVolume>::Create(std::move(centroids), height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetVertices() const
        {
            return mVertices;
        }

        inline std::vector<std::array<std::size_t, 2>> const& GetSegments() const
        {
            return mSegments;
        }

        // Compute intersections of the linear component and leaf nodes. The
        // indices[] are lookups into the mNodes[] member of the base class.
        // The nodeIndices are ordered according to the depth-first traversal
        // of the tree.
        void Execute(
            std::uint32_t queryType,
            Vector3<T> const& P,
            Vector3<T> const& Q,
            std::vector<std::size_t>& nodeIndices)
        {
            this->GetLeafIndices(queryType, P, Q, nodeIndices);
        }

    protected:
        std::vector<Vector3<T>> mVertices;
        std::vector<std::array<size_t, 2>> mSegments;

    private:
        friend class UnitTestBVTreeOfSegments;
    };
}
