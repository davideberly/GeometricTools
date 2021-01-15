// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.09.29

#pragma once

#include <Applications/Window3.h>
#include <MathematicsGPU/GPUFluid3.h>
using namespace gte;

class Fluids3DWindow3 : public Window3
{
public:
    Fluids3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

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
