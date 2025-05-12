// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window2.h>
#include <MathematicsGPU/GPUFluid2.h>
using namespace gte;

//#define SAVE_RENDERING_TO_DISK

class Fluids2DWindow2 : public Window2
{
public:
    Fluids2DWindow2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnResize(int32_t xSize, int32_t ySize) override;

private:
    bool SetEnvironment();
    bool CreateOverlay();

    enum { GRID_SIZE = 256 };
    std::shared_ptr<Shader> mDrawDensityShader;
    std::shared_ptr<OverlayEffect> mOverlay;
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mNoCullingState;
    GPUFluid2 mFluid;

#if defined(SAVE_RENDERING_TO_DISK)
    std::shared_ptr<DrawTarget> mTarget;
    int32_t mVideoFrame;
#endif
};

