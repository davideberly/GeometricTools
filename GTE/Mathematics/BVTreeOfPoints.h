// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.08

#pragma once

// Read the comments in BVTree.h regarding tree construction. Although this
// class appears to be non-abstract, the BoundingVolume type has requirements
// for its interface. In this sense, BVTreeOfPoints is abstract.

#include <Mathematics/BVTree.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace gte
{
    template <typename T, typename BoundingVolume>
    class BVTreeOfPoints : public BVTree<T, BoundingVolume>
    {
    public:
        BVTreeOfPoints()
            :
            BVTree<T, BoundingVolume>{},
            mVertices{}
        {
        }

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<std::size_t>::max(),
        // the entire tree is built and the actual height is computed from
        // vertices.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& vertices,
            std::size_t height = std::numeric_limits<std::size_t>::max())
        {
            LogAssert(
                vertices.size() > 0,
                "Expecting vertices to create a bounding volume tree.");

            mVertices = vertices;

            // The vertices are already the centroids.
            std::vector<Vector3<T>> centroids = vertices;

            // Create the bounding volume tree for centroids.
            this->Tree::Create(std::move(centroids), height);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetVertices() const
        {
            return mVertices;
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

    private:
        friend class UnitTestBVTreeOfPoints;
    };
}
