// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.03.25

#pragma once

// TODO: This is not a KD-tree nearest neighbor query. Instead, it is an
// algorithm to get "approximate" nearest neighbors. Replace this by the
// actual KD-tree query.

// Use a kd-tree for sorting used in a query for finding nearest neighbors of
// a point in a space of the specified dimension N. The split order is always
// 0,1,2,...,N-1. The number of sites at a leaf node is controlled by
// 'maxLeafSize' and the maximum level of the tree is controlled by
// 'maxLevels'. The points are of type Vector<N,T>. The 'Site' is a structure
// of information that minimally implements the function
// 'Vector<N,T> GetPosition() const'. The Site template parameter allows the
// query to be applied even when it has more local information than just point
// location.

#include <Mathematics/Logger.h>
#include <Mathematics/Vector.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

namespace gte
{
    // Predefined site structs for convenience.
    template <int32_t N, typename T>
    struct PositionSite
    {
        Vector<N, T> position;

        PositionSite()
            :
            position{}
        {
            position.MakeZero();
        }

        PositionSite(Vector<N, T> const& p)
            :
            position(p)
        {
        }

        Vector<N, T> GetPosition() const
        {
            return position;
        }
    };

    // Predefined site structs for convenience.
    template <int32_t N, typename T>
    struct PositionDirectionSite
    {
        Vector<N, T> position;
        Vector<N, T> direction;

        PositionDirectionSite()
            :
            position{},
            direction{}
        {
            position.MakeZero();
            direction.MakeZero();
        }

        PositionDirectionSite(Vector<N, T> const& p, Vector<N, T> const& d)
            :
            position(p),
            direction(d)
        {
        }

        Vector<N, T> GetPosition() const
        {
            return position;
        }
    };

    template <int32_t N, typename T, typename Site>
    class NearestNeighborQuery
    {
    public:
        // Supporting data structures.
        using SortedPoint = std::pair<Vector<N, T>, int32_t>;

        struct Node
        {
            T split;
            int32_t axis;
            int32_t numSites;
            int32_t siteOffset;
            int32_t left;
            int32_t right;
        };

        // Construction.
        NearestNeighborQuery(std::vector<Site> const& sites, int32_t maxLeafSize, int32_t maxLevel)
            :
            mMaxLeafSize(maxLeafSize),
            mMaxLevel(maxLevel),
            mSortedPoints(sites.size()),
            mDepth(0),
            mLargestNodeSize(0)
        {
            LogAssert(mMaxLevel > 0 && mMaxLevel <= 32, "Invalid max level.");

            int32_t const numSites = static_cast<int32_t>(sites.size());
            for (int32_t i = 0; i < numSites; ++i)
            {
                mSortedPoints[i] = std::make_pair(sites[i].GetPosition(), i);
            }

            mNodes.push_back(Node());
            Build(numSites, 0, 0, 0);
        }

        // Member access.
        inline int32_t GetMaxLeafSize() const
        {
            return mMaxLeafSize;
        }

        inline int32_t GetMaxLevel() const
        {
            return mMaxLevel;
        }

        inline int32_t GetDepth() const
        {
            return mDepth;
        }

        inline int32_t GetLargestNodeSize() const
        {
            return mLargestNodeSize;
        }

        inline int32_t GetNumNodes() const
        {
            return static_cast<int32_t>(mNodes.size());
        }

        inline std::vector<Node> const& GetNodes() const
        {
            return mNodes;
        }

        // Compute up to MaxNeighbors nearest neighbors within the specified
        // radius of the point. The returned integer is the number of
        // neighbors found, possibly zero. The neighbors array stores indices
        // into the array passed to the constructor. When MaxNeighbors is
        // large and the number of queries is large, performance is better
        // when using a std::priority_queue.
        template <int32_t MaxNeighbors>
        int32_t FindNeighbors(Vector<N, T> const& point, T const& radius,
            std::array<int32_t, MaxNeighbors>& neighbors) const
        {
            static_assert(MaxNeighbors >= 1, "Invalid maximum number of neighbors.");

            T sqrRadius = radius * radius;
            NNPriorityQueue maxHeap{};

            // The kd-tree construction is recursive, simulated here by using
            // a stack. The maximum depth is limited to 32, because the number
            // of sites is limited to 2^{32} (the number of 32-bit integer
            // indices).
            std::array<int32_t, 32> stack{};
            int32_t top = 0;
            stack[0] = 0;

            while (top >= 0)
            {
                Node node = mNodes[stack[top--]];

                if (node.siteOffset != -1)
                {
                    for (int32_t i = 0, j = node.siteOffset; i < node.numSites; ++i, ++j)
                    {
                        Vector<N, T> diff = mSortedPoints[j].first - point;
                        T sqrLength = Dot(diff, diff);
                        if (sqrLength <= sqrRadius)
                        {
                            // Keep track of the nearest neighbors.
                            if (maxHeap.size() < MaxNeighbors)
                            {
                                maxHeap.push(std::make_pair(sqrLength, mSortedPoints[j].second));
                            }
                            else if (sqrLength < maxHeap.top().first)
                            {
                                maxHeap.pop();
                                maxHeap.push(std::make_pair(sqrLength, mSortedPoints[j].second));
                            }
                        }
                    }
                }

                if (node.left != -1 && point[node.axis] - radius <= node.split)
                {
                    stack[++top] = node.left;
                }

                if (node.right != -1 && point[node.axis] + radius >= node.split)
                {
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
// Microsoft Visual Studio 2022 (17.9.4) generates warning C28020 for the next
// line of code. The warning is
// "The expression '0<=_Param(1)&&_Param(1)<=32-1' is not true at this call."
// I believe the analyzer decides that the stack[] can overflow. However,
// the constructor for this class has a LogAssert that mMaxLevel <= 32. This
// condition ensures that the stack cannot have more than 32 elements, but
// the analyzer is not able to infer this.
#endif
                    stack[++top] = node.right;
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                }
            }

            size_t nidx = 0;
            int32_t numNeighbors = static_cast<int32_t>(maxHeap.size());
            while (!maxHeap.empty())
            {
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(disable : 28020)
// Microsoft Visual Studio 2022 (17.9.4) generates warning C28020 for the next
// line of code. The warning is
// "The expression '0<=_Param(1)&&_Param(1)<=1-1' is not true at this call."
// It is not clear what the analyzer is complaining about.
#endif
                neighbors[nidx++] = maxHeap.top().second;
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                maxHeap.pop();
            }

            // TODO: Removing the final set of items from the heap can be a
            // major bottleneck when the number of neighbors is large, say
            // 256 or larger. Evaluate using this method of copying instead.
            // 
            //   int32_t numNeighbors = static_cast<int32_t>(maxHeap.size());
            //   std::vector<VIPair> const& container = maxHeap.GetContainer();
            //   for (size_t i = 0; i < maxHeap.size(); ++i)
            //   {
            //       neighbors[i] = container[i].second;
            //   }

            return numNeighbors;
        }

        inline std::vector<SortedPoint> const& GetSortedPoints() const
        {
            return mSortedPoints;
        }

    private:
        using VIPair = std::pair<T, int32_t>;

        // See the comments in FindNeighbors about an alternative to copying
        // the std::priority_queue elements to the neighbors[] array. The
        // underlying container of std::priority_queue is protected, so for
        // portability, a derived class must be used to expose that container.
        class NNPriorityQueue : public std::priority_queue<VIPair>
        {
        public:
            std::vector<VIPair> const& GetContainer() const
            {
                return this->c;
            }
        };

        // Populate the node so that it contains the points split along the
        // coordinate axes.
        void Build(int32_t numSites, int32_t siteOffset, int32_t nodeIndex, int32_t level)
        {
            LogAssert(siteOffset != -1, "Invalid site offset.");
            LogAssert(nodeIndex != -1, "Invalid node index.");
            LogAssert(numSites > 0, "Empty point list.");

            mDepth = std::max(mDepth, level);

            Node& node = mNodes[nodeIndex];
            node.numSites = numSites;

            if (numSites > mMaxLeafSize && level <= mMaxLevel)
            {
                int32_t halfNumSites = numSites / 2;

                // The point set is too large for a leaf node, so split it at
                // the median.  The O(m log m) sort is not needed; rather, we
                // locate the median using an order statistic construction
                // that is expected time O(m).
                int32_t const axis = level % N;
                auto sorter = [axis](SortedPoint const& p0, SortedPoint const& p1)
                {
                    return p0.first[axis] < p1.first[axis];
                };

                auto begin = mSortedPoints.begin() + siteOffset;
                auto mid = mSortedPoints.begin() + siteOffset + halfNumSites;
                auto end = mSortedPoints.begin() + siteOffset + numSites;
                std::nth_element(begin, mid, end, sorter);

                // Get the median position.
                size_t index = static_cast<size_t>(siteOffset) + static_cast<size_t>(halfNumSites);
                node.split = mSortedPoints[index].first[axis];
                node.axis = axis;
                node.siteOffset = -1;

                // Apply a divide-and-conquer step.
                int32_t left = static_cast<int32_t>(mNodes.size());
                int32_t right = left + 1;
                node.left = left;
                node.right = right;
                mNodes.push_back(Node());
                mNodes.push_back(Node());

                int32_t nextLevel = level + 1;
                Build(halfNumSites, siteOffset, left, nextLevel);
                Build(numSites - halfNumSites, siteOffset + halfNumSites, right, nextLevel);
            }
            else
            {
                // The number of points is small enough or we have run out of
                // depth, so make this node a leaf.
                node.split = std::numeric_limits<T>::max();
                node.axis = -1;
                node.siteOffset = siteOffset;
                node.left = -1;
                node.right = -1;

                mLargestNodeSize = std::max(mLargestNodeSize, node.numSites);
            }
        }

        int32_t mMaxLeafSize;
        int32_t mMaxLevel;
        std::vector<SortedPoint> mSortedPoints;
        std::vector<Node> mNodes;
        int32_t mDepth;
        int32_t mLargestNodeSize;
    };
}
