// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <Graphics/DirectionalLightTextureEffect.h>
#include <Graphics/PointLightTextureEffect.h>
using namespace gte;

class LightTextureWindow3 : public Window3
{
public:
    LightTextureWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void UpdateConstants();

    std::shared_ptr<DirectionalLightTextureEffect> mDLTEffect;
    std::shared_ptr<PointLightTextureEffect> mPLTEffect;
    Vector4<float> mLightWorldPosition;
    Vector4<float> mLightWorldDirection;
    std::shared_ptr<Visual> mTerrain;
    bool mUseDirectional;
};

