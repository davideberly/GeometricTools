// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <MathematicsGPU/GPUFluid3.h>
using namespace gte;

class Fluids3DWindow3 : public Window3
{
public:
    Fluids3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    bool CreateNestedBoxes();
    void UpdateConstants();

    enum { GRID_SIZE = 128 };
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mNoCullingState;
    std::shared_ptr<BlendState> mAlphaState;
    std::shared_ptr<ConstantBuffer> mPVWMatrixBuffer;
    std::vector<std::shared_ptr<Visual>> mVisible;
    GPUFluid3 mFluid;
};
