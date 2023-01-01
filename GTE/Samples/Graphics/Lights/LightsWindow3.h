// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/LightEffect.h>
using namespace gte;

class LightsWindow3 : public Window3
{
public:
    LightsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void UseLightType(int32_t type);
    void UpdateConstants();

    std::shared_ptr<RasterizerState> mWireState;

    enum { LDIR, LPNT, LSPT, LNUM };
    enum { GPLN, GSPH, GNUM };
    enum { SVTX, SPXL, SNUM };
    std::shared_ptr<LightEffect> mEffect[LNUM][GNUM][SNUM];
    std::shared_ptr<Visual> mPlane[SNUM], mSphere[SNUM];
    Vector4<float> mLightWorldPosition[2], mLightWorldDirection;
    std::string mCaption[LNUM];
    int32_t mType;
};
