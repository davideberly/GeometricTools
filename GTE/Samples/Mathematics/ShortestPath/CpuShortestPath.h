// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Texture2.h>
#include <Mathematics/Array2.h>
#include <stack>
using namespace gte;

class CpuShortestPath
{
public:
    CpuShortestPath(std::shared_ptr<Texture2> const& weights);
    void Compute(std::stack<std::pair<int32_t, int32_t>>& path);

private:
    // The weights texture stores (F(x,y), W1(x,y), W2(x,y), W3(x,y)), where
    // F(x,y) is the height field and the edge weights are
    //   W1(x,y) = W((x,y),(x+1,y))   = (F(x+1,y) + F(x,y))/2
    //   W2(x,y) = W((x,y),(x,y+1))   = (F(x,y+1) + F(x,y))/2
    //   W3(x,y) = W((x,y),(x+1,y+1)) = (F(x+1,y+1) + F(x,y))/sqrt(2)
    struct Weights
    {
        // For more readable code using names rather than indices for
        // components of Vector4<float>
        float h, w1, w2, w3;
    };

    // The minimum distance to pixel at (x,y) and the previous neighbor
    // (xPrevious,yPrevious) that led to this minimum.
    struct Node
    {
        Node(float dist = 0.0f, int32_t xPrev = -1, int32_t yPrev = -1);
        float distance;
        int32_t xPrevious, yPrevious;
    };

    // The 'weights' input is mSize-by-mSize.
    int32_t mSize;

    // Use the Array2 object to access 'weights' using 2-tuple locations.
    Array2<Weights> mWeights;

    // Keep track of the distances and the previous pixel that led to the
    // minimum distance for the current pixel.
    Array2<Node> mNodes;
};
