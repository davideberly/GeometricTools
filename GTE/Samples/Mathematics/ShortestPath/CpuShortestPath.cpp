// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CpuShortestPath.h"

CpuShortestPath::CpuShortestPath(std::shared_ptr<Texture2> const& weights)
    :
    mSize(static_cast<int32_t>(weights->GetWidth())),
    mWeights(mSize, mSize, weights->Get<Weights>()),
    mNodes(mSize, mSize)
{
}

void CpuShortestPath::Compute(std::stack<std::pair<int32_t, int32_t>>& path)
{
    // Compute the distances on the top of the grid.  These are simply partial
    // sums of weights along a linear path.
    float distance = 0.0f;
    mNodes[0][0] = Node(distance, -1, -1);
    for (int32_t x = 1; x < mSize; ++x)
    {
        distance += mWeights[0][x - 1].w1;
        mNodes[0][x] = Node(distance, x - 1, 0);
    }

    // Compute the distances on the left edge of the grid.
    distance = 0.0f;
    for (int32_t y = 1; y < mSize; ++y)
    {
        distance += mWeights[y - 1][0].w2;
        mNodes[y][0] = Node(distance, 0, y - 1);
    }

    // The update function for computing the minimum distance at a node using
    // the three incoming edges from its neighbors.
    std::function<void(int32_t, int32_t)> Update = [this](int32_t x, int32_t y)
    {
        float dmin = mNodes[y][x - 1].distance + mWeights[y][x - 1].w1;
        mNodes[y][x] = Node(dmin, x - 1, y);
        float d = mNodes[y - 1][x].distance + mWeights[y - 1][x].w2;
        if (d < dmin)
        {
            dmin = d;
            mNodes[y][x] = Node(dmin, x, y - 1);
        }
        d = mNodes[y - 1][x - 1].distance + mWeights[y - 1][x - 1].w3;
        if (d < dmin)
        {
            dmin = d;
            mNodes[y][x] = Node(dmin, x - 1, y - 1);
        }
    };

    // Compute the distances on the segments x+y=z.  NOTE:  The construction
    // uses knowledge that the grid is a square.  The logic is slightly more
    // complicated for a nonsquare grid, because you have to know when the
    // segments transition from an endpoint on the left edge to an endpoint
    // on the bottom edge (width > height) or from an endpoint on the top
    // edge to an endpoint on the right edge (width < height).  In the case
    // of a square, the endpoints are on left-top and transition to
    // bottom-right at the same time.
    for (int32_t z = 2; z < mSize; ++z)
    {
        for (int32_t x = 1, y = z - x; y > 0; ++x, --y)
        {
            Update(x, y);
        }
    }
    for (int32_t z = mSize; z <= 2 * (mSize - 1); ++z)
    {
        for (int32_t y = mSize - 1, x = z - y; x < mSize; --y, ++x)
        {
            Update(x, y);
        }
    }

    // Create the path by starting at (mXSize-1,mYSize-1) and following the
    // previous links.
    int32_t x = mSize - 1, y = mSize - 1;
    while (x != -1)
    {
        path.push(std::make_pair(x, y));
        Node n = mNodes[y][x];
        x = n.xPrevious;
        y = n.yPrevious;
    }
}

CpuShortestPath::Node::Node(float dist, int32_t xPrev, int32_t yPrev)
    :
    distance(dist),
    xPrevious(xPrev),
    yPrevious(yPrev)
{
}
