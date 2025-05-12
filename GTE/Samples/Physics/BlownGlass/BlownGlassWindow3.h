// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <MathematicsGPU/GPUFluid3.h>
using namespace gte;

class BlownGlassWindow3 : public Window3
{
public:
    BlownGlassWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateScene();

    std::shared_ptr<BlendState> mMeshBlendState;
    std::shared_ptr<RasterizerState> mMeshRasterizerState;
    std::shared_ptr<DepthStencilState> mMeshDepthStencilState;

    enum { GRID_SIZE = 128 };
    std::shared_ptr<Visual> mMesh;
    std::unique_ptr<GPUFluid3> mFluid;
};

