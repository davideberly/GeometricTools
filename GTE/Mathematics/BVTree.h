// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.01.20

#pragma once

// BVTree is an abstract class for computing a bounding volue tree of a
// collection of primitives. The derived classes are BVTreeOfPoints (point
// primitives), BVTreeOfSegments (line segment primitives) and
// BVTreeOfTriangles (triangle primitives). In turn, derived classes of these
// classes create a boundng volume for each tree node.
//
// The depth of a node in a nonempty tree is the distance from the node to the
// root of the tree. The height is the maximum depth. A tree with a single
// node has height 0. The set of nodes of a tree with the same depth is
// referred to as a level of a tree corresponding to that depth. A complete
// binary tree of height H has 2^{H+1}-1 nodes. The level corresponding to
// depth D has 2^D nodes, in which case the number of leaf nodes (nodes at
// depth H) is 2^H.
// 
// The partitioning of primitives between left and right children of a node
// is based on the projection of centroids of the primitives onto a line
// determined by the bounding volume type. The median of projections is chosen
// to partition the primitives into two subsets of equal size or absolute size
// difference of 1. This leads to a balanced tree, which is helpful for
// performance of tree traversals.

#include <Mathematics/BitHacks.h>
#include <Mathematics/Vector3.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

namespace gte
{
    // The interface of class BoundingVolume must include
    //   BoundingVolume();
    // 
    //   ~BoundingVolume();
    // 
    //   void GetSplittingAxis(Vector3<T>& origin, Vector3<T>& direction);
    // 
    //   static bool IntersectLine(
    //       Vector3<T> const& P, Vector3<T> const& Q,
    //       BoundingVolume& boundingVolume);
    // 
    //   static bool IntersectRay(
    //       Vector3<T> const& P, Vector3<T> const& Q,
    //       BoundingVolume& boundingVolume);
    // 
    //   static bool IntersectSegment(
    //       Vector3<T> const& P, Vector3<T> const& Q,
    //       BoundingVolume& boundingVolume);

    template <typename T, typename BoundingVolume>
    class BVTree
    {
    protected:
        struct Node
        {
            Node()
                :
                boundingVolume{},
                minIndex(std::numeric_limits<size_t>::max()),
                maxIndex(std::numeric_limits<size_t>::max()),
                leftChild(std::numeric_limits<size_t>::max()),
                rightChild(std::numeric_limits<size_t>::max())
            {

            }

            BoundingVolume boundingVolume;
            size_t minIndex, maxIndex;
            size_t leftChild, rightChild;
        };

        // Abstract base class.
        BVTree()
            :
            mCentroids{},
            mHeight(0),
            mNodes{},
            mPartition{}
        {
        }

    public:
        virtual ~BVTree() = default;

        // The derived classes must compute the centroids of the primitives
        // and store them in the member mCentroids before calling create.
        //
        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(size_t height)
        {
            LogAssert(mCentroids.size() > 0, "Invalid input.");

            if (height == std::numeric_limits<size_t>::max())
            {
                uint64_t minPowerOfTwo = BitHacks::RoundUpToPowerOfTwo(
                    static_cast<uint32_t>(mCentroids.size()));
                uint32_t logMinPowerOfTwo = BitHacks::Log2OfPowerOfTwo(
                    static_cast<uint32_t>(minPowerOfTwo));
                mHeight = static_cast<size_t>(logMinPowerOfTwo);
            }
            else
            {
                mHeight = std::min(height, static_cast<size_t>(31));
            }

            // The tree is built recursively. Preallocate the nodes because
            // the BuiltTree function declares references on the stack, so
            // we must guarantee that no reallocations occur in order to avoid
            // invalidating those references.
            size_t const numNodes = (static_cast<size_t>(1) << (mHeight + 1)) - 1;
            mNodes.resize(numNodes);

            // The array mPartition stores indices into mCentroids so that at
            // a node, the centroids represented by the node are the indices
            // [mPartition[node.minIndex], mPartition[node.maxIndex]].
            mPartition.resize(mCentroids.size());
            std::iota(mPartition.begin(), mPartition.end(), 0);

            // Build the tree recursively.
            size_t const depth = 0;
            size_t const nodeIndex = 0;
            size_t const i0 = 0;
            size_t const i1 = mCentroids.size() - 1;
            BuildTree(depth, nodeIndex, i0, i1);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetCentroids() const
        {
            return mCentroids;
        }

        inline size_t GetHeight() const
        {
            return mHeight;
        }

        inline std::vector<Node> const& GetNodes() const
        {
            return mNodes;
        }

        inline std::vector<size_t> const& GetPartition() const
        {
            return mPartition;
        }

    protected:
        // The bounding volume for the primitives' vertices depends on the
        // type of primitive. A derived class representing a primitive tree
        // must implement this.
        virtual void ComputeInteriorBoundingVolume(size_t i0, size_t i1,
            BoundingVolume& boundingVolume) = 0;

        // The bounding volume for a single primitive's vertices depends on
        // the type of primitive. A derived class representing a primitive
        // tree must implement this.
        virtual void ComputeLeafBoundingVolume(size_t i,
            BoundingVolume& boundingVolume) = 0;

        std::vector<Vector3<T>> mCentroids;
        size_t mHeight;
        std::vector<Node> mNodes;
        std::vector<size_t> mPartition;

    private:
        void BuildTree(size_t depth, size_t nodeIndex, size_t i0, size_t i1)
        {
            auto& node = mNodes[nodeIndex];
            node.minIndex = i0;
            node.maxIndex = i1;

            if (i0 < i1)
            {
                // The node is interior. Compute a bounding volume for the
                // primitives' vertices.
                ComputeInteriorBoundingVolume(i0, i1, node.boundingVolume);
                if (depth == mHeight)
                {
                    // The user-specified height has been reached. Do not
                    // continue the recursion past this node.
                    return;
                }

                // The BoundingVolume type provides a function to access a
                // splitting axis, typically one in a direction of largest
                // distribution of primitive vertices. Use the splitting axis
                // to partition the centroids of the primitives into two
                // subsets, one for the left child and one for the right
                // child. The subsets have numbers of elements that differ by
                // at most 1, so the tree is balanced.
                size_t j0{}, j1{};
                SplitPoints(i0, i1, node.boundingVolume, j0, j1);

                // Recurse on the two children.
                node.leftChild = 2 * nodeIndex + 1;
                node.rightChild = node.leftChild + 1;
                BuildTree(depth + 1, node.leftChild, i0, j0);
                BuildTree(depth + 1, node.rightChild, j1, i1);
            }
            else // i0 = i1
            {
                // The node is a leaf. Compute a bounding volume for a single
                // primitive's vertices.
                ComputeLeafBoundingVolume(i0, node.boundingVolume);
            }
        }

        struct ProjectionInfo
        {
            ProjectionInfo()
                :
                centroidIndex(0),
                projection(static_cast<T>(0))
            {
            }

            ProjectionInfo(size_t inCentroidIndex, T const& inProjection)
                :
                centroidIndex(inCentroidIndex),
                projection(inProjection)
            {
            }

            bool operator<(ProjectionInfo const& info) const
            {
                return projection < info.projection;
            }

            size_t centroidIndex;
            T projection;
        };

        void SplitPoints(size_t i0, size_t i1, BoundingVolume const& boundingVolume,
            size_t& j0, size_t& j1)
        {
            // The direction of the splitting axis is provided by the
            // BoundingVolume type.
            Vector3<T> origin{}, direction{};
            boundingVolume.GetSplittingAxis(origin, direction);

            // Project the centroids onto the splitting axis.
            size_t const numProjections = i1 - i0 + 1;
            std::vector<ProjectionInfo> info{};
            info.reserve(numProjections);
            for (size_t i = i0; i <= i1; ++i)
            {
                size_t centroidIndex = mPartition[i];
                Vector3<T> diff = mCentroids[centroidIndex] - origin;
                T projection = Dot(direction, diff);
                info.emplace_back(centroidIndex, projection);
            }

            // Partition the projections by the median.
            size_t medianIndex = (numProjections - 1) / 2;
            std::nth_element(info.begin(), info.begin() + medianIndex, info.end());

            // Partition the centroids by the median.
            size_t k{};
            for (k = 0, j0 = i0 - 1; k <= medianIndex; ++k)
            {
                mPartition[++j0] = info[k].centroidIndex;
            }
            for (j1 = i1 + 1; k < numProjections; ++k)
            {
                mPartition[--j1] = info[k].centroidIndex;
            }
        }
    };
}
