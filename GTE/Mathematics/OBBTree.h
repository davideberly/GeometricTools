// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.07.14

#pragma once

// OBBTree is an abstract class for computing an oriented bounding box tree of
// a collection of primitives. The derived classes are OBBTreeOfPoints
// (point primitives), OBBTreeOfSegments (line segment primitives) and
// OBBTreeOfTriangles (triangle primitives). The derived classes create a box
// for each tree node. The box center is the mean of centroids of the
// primitives that the node represents. The box axis directions are the
// eigenvectors of the covariance matrix of those centroids. The box extents
// are computed to ensure the box contains the primitives represented by the
// node.
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
// determined by eigenvectors corresponding to the largest eigenvalue of
// covariance matrices. The median of projections is chosen to partition the
// primitives into two subsets of equal size or absolute size difference of 1.
// This leads to a balanced tree, which is helpful for performance of tree
// traversals.

#include <Mathematics/BitHacks.h>
#include <Mathematics/SymmetricEigensolver3x3.h>
#include <Mathematics/OrientedBox.h>
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
    template <typename T>
    struct OBBNode
    {
    public:
        OBBNode()
            :
            box{},
            minIndex(std::numeric_limits<size_t>::max()),
            maxIndex(std::numeric_limits<size_t>::max()),
            leftChild(std::numeric_limits<size_t>::max()),
            rightChild(std::numeric_limits<size_t>::max())
        {
        }

        OrientedBox3<T> box;
        size_t minIndex, maxIndex;
        size_t leftChild, rightChild;
    };

    template <typename T>
    class OBBTree
    {
    protected:
        // Abstract base class.
        OBBTree()
            :
            mCentroids{},
            mHeight(0),
            mNodes{},
            mPartition{}
        {
        }

    public:
        virtual ~OBBTree() = default;

        // The input height specifies the desired height of the tree and must
        // be no larger than 31. If std::numeric_limits<size_t>::max(), the
        // the entire tree is built and the actual height is computed from
        // centroids.size(). If larger than 31, the height is clamped to 31.
        void Create(
            std::vector<Vector3<T>> const& centroids,
            size_t height)
        {
            LogAssert(centroids.size() > 0, "Invalid input.");
            mCentroids = centroids;

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

            // The tree is built recursively. A reference to an OBBNode is
            // passed to BuildTree and nodes are appended to a std::vector.
            // Because the references are on the stack, we must guarantee
            // that no reallocations occur in order to avoid invalidating
            // references.
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

        inline std::vector<OBBNode<T>> const& GetNodes() const
        {
            return mNodes;
        }

        inline std::vector<size_t> const& GetPartition() const
        {
            return mPartition;
        }

    protected:
        // The derived classes must override ComputeInteriorBox, calling the
        // base class function first. They must then compute the box extents
        // to ensure the box contains the primitives represented by the node.
        virtual void ComputeInteriorBox(size_t i0, size_t i1, OrientedBox3<T>& box)
        {
            // Compute the mean of the centroids.
            Vector3<T> const vzero = Vector3<T>::Zero();
            box.center = vzero;
            for (size_t i = i0; i <= i1; ++i)
            {
                box.center += mCentroids[mPartition[i]];
            }
            T denom = static_cast<T>(i1 - i0 + 1);
            box.center /= denom;

            // Compute the covariance matrix of the centroids.
            T const zero = static_cast<T>(0);
            T covar00 = zero, covar01 = zero, covar02 = zero;
            T covar11 = zero, covar12 = zero, covar22 = zero;
            for (size_t i = i0; i <= i1; ++i)
            {
                Vector3<T> diff = mCentroids[mPartition[i]] - box.center;
                covar00 += diff[0] * diff[0];
                covar01 += diff[0] * diff[1];
                covar02 += diff[0] * diff[2];
                covar11 += diff[1] * diff[1];
                covar12 += diff[1] * diff[2];
                covar22 += diff[2] * diff[2];
            }
            covar00 /= denom;
            covar01 /= denom;
            covar02 /= denom;
            covar11 /= denom;
            covar12 /= denom;
            covar22 /= denom;

            // Use the eigenvectors of the covariance matrix for the box axes.
            SymmetricEigensolver3x3<T> es{};
            std::array<T, 3> eval{};
            std::array<std::array<T, 3>, 3> evec{};
            es(covar00, covar01, covar02, covar11, covar12, covar22, false, +1, eval, evec);
            for (size_t i = 0; i < 3; ++i)
            {
                box.axis[i] = evec[i];
            }

            // The box.extent values must be computed by the derived classes.
            // For debugging, store the eigenvalues in the extents.
            box.extent = eval;
        }

        // The derived classes must override ComputeLeafBox. The intrinsic box
        // dimension depends on the geometric primitive.
        virtual void ComputeLeafBox(size_t i, OrientedBox3<T>& box) = 0;

        std::vector<Vector3<T>> mCentroids;
        size_t mHeight;
        std::vector<OBBNode<T>> mNodes;
        std::vector<size_t> mPartition;

    private:
        void BuildTree(size_t depth, size_t nodeIndex, size_t i0, size_t i1)
        {
            auto& node = mNodes[nodeIndex];
            node.minIndex = i0;
            node.maxIndex = i1;

            if (i0 < i1)
            {
                // The node is interior. Compute an oriented bounding box of
                // centroids, but then with extents modified to ensure the box
                // contains the primitives represented by the node.
                ComputeInteriorBox(i0, i1, node.box);
                if (depth == mHeight)
                {
                    // The user-specified height has been reached. Do not
                    // continue the recursion past this node.
                    return;
                }

                // Use the box axis corresponding to largest extent for the
                // splitting axis. Partition the centroids into two subsets,
                // one for the left child and one for the right child. The
                // subsets have numbers of elements that differ by at most 1,
                // so the tree is balanced.
                size_t j0{}, j1{};
                SplitPoints(i0, i1, node.box.center, node.box.axis[2], j0, j1);

                // Recurse on the two children.
                node.leftChild = 2 * nodeIndex + 1;
                node.rightChild = node.leftChild + 1;
                BuildTree(depth + 1, node.leftChild, i0, j0);
                BuildTree(depth + 1, node.rightChild, j1, i1);
            }
            else // i0 = i1
            {
                // The node is a leaf. Compute a primitive-dependent oriented
                // bounding box.
                ComputeLeafBox(i0, node.box);
            }
        }

        struct ProjectionInfo
        {
            ProjectionInfo()
                :
                pointIndex(0),
                projection(static_cast<T>(0))
            {
            }

            ProjectionInfo(size_t inPointIndex, T const& inProjection)
                :
                pointIndex(inPointIndex),
                projection(inProjection)
            {
            }

            bool operator<(ProjectionInfo const& info) const
            {
                return projection < info.projection;
            }

            size_t pointIndex;
            T projection;
        };

        void SplitPoints(size_t i0, size_t i1, Vector3<T> const& origin,
            Vector3<T> const& direction, size_t& j0, size_t& j1)
        {
            // Project the centroids onto the splitting axis.
            size_t const numProjections = i1 - i0 + 1;
            std::vector<ProjectionInfo> info{};
            info.reserve(numProjections);
            for (size_t i = i0; i <= i1; ++i)
            {
                size_t pointIndex = mPartition[i];
                Vector3<T> diff = mCentroids[pointIndex] - origin;
                T projection = Dot(direction, diff);
                info.emplace_back(pointIndex, projection);
            }

            // Partition the projections by the median.
            size_t medianIndex = (numProjections - 1) / 2;
            std::nth_element(info.begin(), info.begin() + medianIndex, info.end());

            // Partition the centroids by the median.
            size_t k{};
            for (k = 0, j0 = i0 - 1; k <= medianIndex; ++k)
            {
                mPartition[++j0] = info[k].pointIndex;
            }
            for (j1 = i1 + 1; k < numProjections; ++k)
            {
                mPartition[--j1] = info[k].pointIndex;
            }
        }
    };
}
