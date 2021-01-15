// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include "BloodCellController.h"
using namespace gte;

class ParticleControllersWindow3 : public Window3
{
public:
    ParticleControllersWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void CreateScene();

    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<DepthStencilState> mNoDepthState;
    std::shared_ptr<RasterizerState> mWireState;

    std::shared_ptr<Particles> mParticles;
    std::shared_ptr<BloodCellController> mBloodCellController;
    Timer mApplicationTimer;
};
