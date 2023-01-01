// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Environment.h>
#include <Graphics/Graphics.h>
#include <stack>
using namespace gte;

class GpuShortestPath
{
public:
    GpuShortestPath(std::shared_ptr<GraphicsEngine> const& engine,
        std::shared_ptr<ProgramFactory> const& factory,
        std::shared_ptr<Texture2> const& weights, Environment const& env,
        bool& created);

    void Compute(std::shared_ptr<GraphicsEngine> const& engine,
        std::stack<std::pair<int32_t, int32_t>>& path);

private:
    // The 'weights' input is mSize-by-mSize.
    int32_t mSize, mLogSize;

    std::shared_ptr<Texture2> mDistance, mPrevious;
    std::shared_ptr<ConstantBuffer> mSegment;

    std::shared_ptr<ComputeProgram> mInitializeDiagToRow;
    std::shared_ptr<ComputeProgram> mInitializeDiagToCol;
    std::vector<std::shared_ptr<ComputeProgram>> mPartialSumDiagToRow;
    std::vector<std::shared_ptr<ComputeProgram>> mPartialSumDiagToCol;
    std::shared_ptr<ComputeProgram> mUpdate;
};
