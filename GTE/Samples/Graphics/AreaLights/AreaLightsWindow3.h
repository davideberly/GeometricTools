// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <Graphics/AreaLightEffect.h>
using namespace gte;

class AreaLightsWindow3 : public Window3
{
public:
    AreaLightsWindow3(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateSurface();
    void CreateAreaLightEffect();
    void UpdateConstants();

    std::shared_ptr<Visual> mSurface;
    std::shared_ptr<Texture2> mSurfaceTexture, mNormalTexture;
    Vector4<float> mALWorldPosition, mALWorldNormal, mALWorldAxis0, mALWorldAxis1;
    Vector4<float> mALExtent;
    std::shared_ptr<AreaLightEffect> mALEffect;
};

