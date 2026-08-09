// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.3.2026.08.09

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
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

namespace gte
{
    // The interface of class BoundingVolume must include
    //
    // template <typename T>
    // class BoundingVolume
    // {
    // public:
    //     BoundingVolume();
    //     ~BoundingVolume();
    //     void GetSplittingAxis(Vector3<T>& origin, Vector3<T>& direction);
    //     static bool IntersectLine(Vector3<T> const& P, Vector3<T> const& Q, BoundingVolume<T> const& boundingVolume);
    //     static bool IntersectRay(Vector3<T> const& P, Vector3<T> const& Q, BoundingVolume<T> const& boundingVolume);
    //     static bool IntersectSegment(Vector3<T> const& P, Vector3<T> const& Q, BoundingVolume<T> const& boundingVolume);
    // };
    // The line is parameterized by P+t*Q for all real t. The ray is
    // parameterized by P+t*Q for nonnegative t. The segment is parameterized
    // by (1-t)*P+t*Q for t in [0,1].
    //
    // To support building the tree, derived classes of BVTree must implement
    // virtual functions
    //     void ComputeInteriorBoundingVolume(std::size_t i0, std::size_t i1, BoundingVolume& boundingVolume);
    //     void ComputeLeafBoundingVolume(std::size_t i, BoundingVolume& boundingVolume);

    template <typename T, typename BoundingVolume>
    class BVTree
    {
    protected:
        // Convenient alias for derived classes.
        using Tree = BVTree<T, BoundingVolume>;

        // Function signature for {line,ray,segment}-boundingVolume
        // test-intersection queries.
        using LinearBoundingVolumeQuery = bool (*)(Vector3<T> const&,
            Vector3<T> const&, BoundingVolume const&);

        // Abstract base class. The derived classes must compute the centroids
        // of the primitives and store them in the member mCentroids before
        // calling the Create(...) function.
        BVTree()
            :
            mCentroids{},
            mHeight(0),
            mNodes{},
            mPartition{},
            mLinearBoundingVolumeQuery{
                BoundingVolume::IntersectLine,
                BoundingVolume::IntersectRay,
                BoundingVolume::IntersectSegment
            }
        {
        }

    public:
        // These are the queryType inputs to the derived classes Execute(...)
        // functions. Generate a list of leaf nodes intersected by a linear
        // component (line, ray or segment). The line is parameterized by
        // P + t * Q, where Q is a unit-length direction and t is any real
        // number. The ray is parameterized by P + t * Q, where Q is a
        // unit-length direction and t >= 0. The segment is parameterized by
        // (1-t) * P + t * Q = P + t * (Q - P), where P and Q are the
        // endpoints of the segment and 0 <= t <= 1.
        static std::uint32_t constexpr LINE_QUERY = 0;
        static std::uint32_t constexpr RAY_QUERY = 1;
        static std::uint32_t constexpr SEGMENT_QUERY = 2;


        class Node
        {
        public:
            static std::size_t constexpr invalid = std::numeric_limits<std::size_t>::max();

            Node()
                :
                boundingVolume{},
                minIndex(std::numeric_limits<std::size_t>::max()),
                maxIndex(std::numeric_limits<std::size_t>::max()),
                leftChild(std::numeric_limits<std::size_t>::max()),
                rightChild(std::numeric_limits<std::size_t>::max())
            {
            }

            BoundingVolume boundingVolume;
            std::size_t minIndex, maxIndex;
            std::size_t leftChild, rightChild;
        };


        virtual ~BVTree() = default;

        // The derived classes must compute the centroids of the primitives
        // and store them in the member mCentroids before calling the
        // Create(...) function.
        //
        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<std::size_t>::max(),
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>>&& centroids,
            std::size_t height)
        {
            LogAssert(
                centroids.size() > 0,
                "Expecting centroids to create a bounding volume tree.");

            mCentroids = std::move(centroids);

            if (height == std::numeric_limits<std::size_t>::max())
            {
                std::uint64_t minPowerOfTwo = BitHacks::RoundUpToPowerOfTwo(
                    static_cast<std::uint32_t>(mCentroids.size()));
                std::uint32_t logMinPowerOfTwo = BitHacks::Log2OfPowerOfTwo(
                    static_cast<std::uint32_t>(minPowerOfTwo));
                mHeight = static_cast<std::size_t>(logMinPowerOfTwo);
            }
            else
            {
                mHeight = std::min(height, static_cast<std::size_t>(31));
            }

            // The tree is built recursively. Preallocate the nodes because
            // the BuildTree function declares references on the stack. We
            // must guarantee that no reallocations occur in order to avoid
            // invalidating those references.
            std::size_t const numNodes = (static_cast<std::size_t>(1) << (mHeight + 1)) - 1;
            mNodes.resize(numNodes);

            // The array mPartition stores indices into mCentroids so that at
            // a node, the centroids represented by the node are the indices
            // [mPartition[node.minIndex], mPartition[node.maxIndex]].
            mPartition.resize(mCentroids.size());
            std::iota(mPartition.begin(), mPartition.end(), 0);

            // Build the tree recursively.
            std::size_t const depth = 0;
            std::size_t const nodeIndex = 0;
            std::size_t const i0 = 0;
            std::size_t const i1 = mCentroids.size() - 1;
            BuildTree(depth, nodeIndex, i0, i1);
        }

        // Member access.
        inline std::vector<Vector3<T>> const& GetCentroids() const
        {
            return mCentroids;
        }

        inline std::size_t GetHeight() const
        {
            return mHeight;
        }

        inline std::vector<Node> const& GetNodes() const
        {
            return mNodes;
        }

        inline std::vector<std::size_t> const& GetPartition() const
        {
            return mPartition;
        }

    protected:
        // The bounding volume for the primitives' vertices depends on the
        // type of primitive. A derived class representing a primitive tree
        // must implement this.
        virtual void ComputeInteriorBoundingVolume(
            std::size_t i0,
            std::size_t i1,
            BoundingVolume& boundingVolume) = 0;

        // The bounding volume for a single primitive's vertices depends on
        // the type of primitive. A derived class representing a primitive
        // tree must implement this.
        virtual void ComputeLeafBoundingVolume(
            std::size_t i,
            BoundingVolume& boundingVolume) = 0;

        // Get the node indices for the leaf nodes whose bounding volumes are
        // intersected by the linear component.
        void GetLeafIndices(
            std::uint32_t queryType,
            Vector3<T> const& P,
            Vector3<T> const& Q,
            std::vector<std::size_t>& nodeIndices)
        {
            nodeIndices.clear();

            auto linearBoundaryVolumeQuery = this->mLinearBoundingVolumeQuery[queryType];
            std::vector<std::size_t> indexStack(2 * this->mHeight + 1);
            std::size_t top = 0;
            indexStack[0] = 0;
            while (top != std::numeric_limits<std::size_t>::max())
            {
                std::size_t nodeIndex = indexStack[top--];
                auto const& node = mNodes[nodeIndex];

                // For the balanced tree created by BVTree<T>, an interior
                // node has two valid children and a leaf node has two invalid
                // children. This is true even if the height passed to
                // BVTree<T>::Create is smaller than the actual height.
                if (node.leftChild != Node::invalid &&
                    node.rightChild != Node::invalid)
                {
                    // The node is interior.
                    if (linearBoundaryVolumeQuery(P, Q, node.boundingVolume))
                    {
                        // The linear component intersects the box. Continue
                        // the intersection search to child nodes if they
                        // exist.
                        indexStack[++top] = node.rightChild;
                        indexStack[++top] = node.leftChild;
                    }
                    else
                    {
                        // The linear component does not intersect the box. Do
                        // not continue the intersection search to child nodes
                        // if they exist.
                    }
                }
                else // node.leftChild == invalid && node.rightChild == invalid
                {
                    nodeIndices.push_back(nodeIndex);
                }
            }
        }

        std::vector<Vector3<T>> mCentroids;
        std::size_t mHeight;
        std::vector<Node> mNodes;
        std::vector<std::size_t> mPartition;
        std::array<LinearBoundingVolumeQuery, 3> mLinearBoundingVolumeQuery;

    private:
        // Support for tree creation.
        void BuildTree(
            std::size_t depth,
            std::size_t nodeIndex,
            std::size_t i0,
            std::size_t i1)
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
                std::size_t j0{}, j1{};
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

        class ProjectionInfo
        {
        public:
            ProjectionInfo()
                :
                centroidIndex(0),
                projection(static_cast<T>(0))
            {

            }

            ProjectionInfo(
                std::size_t inCentroidIndex,
                T const& inProjection)
                :
                centroidIndex(inCentroidIndex),
                projection(inProjection)
            {
            }

            bool operator<(ProjectionInfo const& info) const
            {
                return projection < info.projection;
            }

            std::size_t centroidIndex;
            T projection;
        };

        void SplitPoints(
            std::size_t i0,
            std::size_t i1,
            BoundingVolume const& boundingVolume,
            std::size_t& j0,
            std::size_t& j1)
        {
            // The direction of the splitting axis is provided by the
            // BoundingVolume type.
            Vector3<T> origin{}, direction{};
            boundingVolume.GetSplittingAxis(origin, direction);

            // Project the centroids onto the splitting axis.
            std::size_t const numProjections = i1 - i0 + 1;
            std::vector<ProjectionInfo> info{};
            info.reserve(numProjections);
            for (std::size_t i = i0; i <= i1; ++i)
            {
                std::size_t centroidIndex = mPartition[i];
                Vector3<T> diff = mCentroids[centroidIndex] - origin;
                T projection = Dot(direction, diff);
                info.emplace_back(centroidIndex, projection);
            }

            // Partition the projections by the median.
            std::size_t medianIndex = (numProjections - 1) / 2;
            std::nth_element(info.begin(), info.begin() + medianIndex, info.end());

            // Partition the centroids by the median. When i0 = 0, the initial
            // j0 is the maximum of std::size_t. However, it is incremented
            // to 0 before the lookup into mPartition[]. The wrap-around is
            // allowed by C/C++.
            std::size_t k{};
            for (k = 0, j0 = i0 - 1; k <= medianIndex; ++k)
            {
                mPartition[++j0] = info[k].centroidIndex;
            }
            for (j1 = i1 + 1; k < numProjections; ++k)
            {
                mPartition[--j1] = info[k].centroidIndex;
            }
        }

    private:
        friend class UnitTestBVTree;
    };
}
