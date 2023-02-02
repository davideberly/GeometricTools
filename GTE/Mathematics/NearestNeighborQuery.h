// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Logger.h>
#include <Mathematics/Vector.h>
#include <vector>

// TODO: This is not a KD-tree nearest neighbor query. Instead, it is an
// algorithm to get "approximate" nearest neighbors. Replace this by the
// actual KD-tree query.

// Use a kd-tree for sorting used in a query for finding nearest neighbors of
// a point in a space of the specified dimension N. The split order is always
// 0,1,2,...,N-1. The number of sites at a leaf node is controlled by
// 'maxLeafSize' and the maximum level of the tree is controlled by
// 'maxLevels'. The points are of type Vector<N,Real>. The 'Site' is a
// structure of information that minimally implements the function
// 'Vector<N,Real> GetPosition () const'. The Site template parameter
// allows the query to be applied even when it has more local information
// than just point location.

namespace gte
{
    // Predefined site structs for convenience.
    template <int32_t N, typename T>
    struct PositionSite
    {
        Vector<N, T> position;

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

    template <int32_t N, typename Real, typename Site>
    class NearestNeighborQuery
    {
    public:
        // Supporting data structures.
        typedef std::pair<Vector<N, Real>, int32_t> SortedPoint;

        struct Node
        {
            Real split;
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

        int32_t GetNumNodes() const
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
        // into the array passed to the constructor.
        template <int32_t MaxNeighbors>
        int32_t FindNeighbors(Vector<N, Real> const& point, Real radius, std::array<int32_t, MaxNeighbors>& neighbors) const
        {
            Real sqrRadius = radius * radius;
            int32_t numNeighbors = 0;
            std::array<int32_t, MaxNeighbors + 1> localNeighbors;
            std::array<Real, MaxNeighbors + 1> neighborSqrLength;
            for (int32_t i = 0; i <= MaxNeighbors; ++i)
            {
                localNeighbors[i] = -1;
                neighborSqrLength[i] = std::numeric_limits<Real>::max();
            }

            // The kd-tree construction is recursive, simulated here by using
            // a stack. The maximum depth is limited to 32, because the number
            // of sites is limited to 2^{32} (the number of 32-bit integer
            // indices).
            std::array<int32_t, 32> stack{};
            int32_t top = 0;
            stack[0] = 0;

            int32_t maxNeighbors = MaxNeighbors;
            if (maxNeighbors == 1)
            {
                while (top >= 0)
                {
                    Node node = mNodes[stack[top--]];

                    if (node.siteOffset != -1)
                    {
                        for (int32_t i = 0, j = node.siteOffset; i < node.numSites; ++i, ++j)
                        {
                            auto diff = mSortedPoints[j].first - point;
                            auto sqrLength = Dot(diff, diff);
                            if (sqrLength <= sqrRadius)
                            {
                                // Maintain the nearest neighbors.
                                if (sqrLength <= neighborSqrLength[0])
                                {
                                    localNeighbors[0] = mSortedPoints[j].second;
                                    neighborSqrLength[0] = sqrLength;
                                    numNeighbors = 1;
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
#endif
                        stack[++top] = node.right;
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                    }
                }
            }
            else
            {
                while (top >= 0)
                {
                    Node node = mNodes[stack[top--]];

                    if (node.siteOffset != -1)
                    {
                        for (int32_t i = 0, j = node.siteOffset; i < node.numSites; ++i, ++j)
                        {
                            Vector<N, Real> diff = mSortedPoints[j].first - point;
                            Real sqrLength = Dot(diff, diff);
                            if (sqrLength <= sqrRadius)
                            {
                                // Maintain the nearest neighbors.
                                int32_t k;
                                for (k = 0; k < numNeighbors; ++k)
                                {
                                    if (sqrLength <= neighborSqrLength[k])
                                    {
                                        for (int32_t n = numNeighbors; n > k; --n)
                                        {
                                            localNeighbors[n] = localNeighbors[static_cast<size_t>(n) - 1];
                                            neighborSqrLength[n] = neighborSqrLength[static_cast<size_t>(n) - 1];
                                        }
                                        break;
                                    }
                                }
                                if (k < MaxNeighbors)
                                {
                                    localNeighbors[k] = mSortedPoints[j].second;
                                    neighborSqrLength[k] = sqrLength;
                                }
                                if (numNeighbors < MaxNeighbors)
                                {
                                    ++numNeighbors;
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
#endif
                        stack[++top] = node.right;
#if defined(GTE_USE_MSWINDOWS)
#pragma warning(default : 28020)
#endif
                    }
                }
            }


            for (int32_t i = 0; i < numNeighbors; ++i)
            {
                neighbors[i] = localNeighbors[i];
            }

            return numNeighbors;
        }

        inline std::vector<SortedPoint> const& GetSortedPoints() const
        {
            return mSortedPoints;
        }

    private:
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
                node.split = mSortedPoints[static_cast<size_t>(siteOffset) + static_cast<size_t>(halfNumSites)].first[axis];
                node.axis = axis;
                node.siteOffset = -1;

                // Apply a divide-and-conquer step.
                int32_t left = (int32_t)mNodes.size(), right = left + 1;
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
                node.split = std::numeric_limits<Real>::max();
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
