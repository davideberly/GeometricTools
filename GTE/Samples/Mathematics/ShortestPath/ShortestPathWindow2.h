// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

//#define USE_CPU_SHORTEST_PATH

#include <Applications/Window2.h>
#if defined(USE_CPU_SHORTEST_PATH)
#include "CpuShortestPath.h"
#else
#include "GpuShortestPath.h"
#endif
using namespace gte;

class ShortestPathWindow2 : public Window2
{
public:
    // The application window is square and the height field for the
    // graph is square, both with this dimension.
    enum
    {
        ISIZE = 512,
        LOGISIZE = 9
    };

    ShortestPathWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    bool SetEnvironment();
    bool CreateWeightsShader();
    void GenerateWeights();
    std::shared_ptr<ConstantBuffer> CreateBicubicMatrix();
    void DrawPath(std::stack<std::pair<int32_t, int32_t>>& path);

    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<Texture2> mRandom;
    std::shared_ptr<Texture2> mWeights;
    std::shared_ptr<ComputeProgram> mWeightsProgram;
    uint32_t mNumGroups;

#if defined(USE_CPU_SHORTEST_PATH)
    std::shared_ptr<CpuShortestPath> mCpuShortestPath;
#else
    std::shared_ptr<GpuShortestPath> mGpuShortestPath;
#endif
};
