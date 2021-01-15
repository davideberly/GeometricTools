// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Texture2Effect.h>
#include "BlendedTerrainEffect.h"
using namespace gte;

class BlendedTerrainWindow3 : public Window3
{
public:
    BlendedTerrainWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    bool SetEnvironment();
    bool CreateTerrain();
    void CreateSkyDome();
    void Update();

    std::shared_ptr<Visual> mTerrain;
    std::shared_ptr<Visual> mSkyDome;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendedTerrainEffect> mTerrainEffect;
    std::shared_ptr<Texture2Effect> mSkyDomeEffect;
    float mFlowDelta, mPowerDelta, mZAngle, mZDeltaAngle;
};
