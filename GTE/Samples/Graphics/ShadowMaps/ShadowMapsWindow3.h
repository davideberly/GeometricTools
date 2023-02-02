// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Light.h>
#include "SMBlurEffect.h"
#include "SMSceneEffect.h"
#include "SMShadowEffect.h"
#include "SMUnlitEffect.h"
using namespace gte;

class ShadowMapsWindow3 : public Window3
{
public:
    ShadowMapsWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    void CreateLightProjector();
    void CreateDrawTargets();
    void CreateSceneEffects();
    void CreateShadowEffects();
    void CreateUnlitEffects();
    void CreateBlurEffects();
    void CreateScene();

    void UpdateSceneEffects();
    void UpdateShadowEffects();
    void UpdateUnlitEffects();

    void DrawUsingSceneEffects();
    void DrawUsingShadowEffects();
    void DrawUsingUnlitEffects();
    void ApplyBlur();

    Light mLightProjector;
    uint32_t mShadowTargetSize;
    std::shared_ptr<DrawTarget> mShadowTarget;
    std::shared_ptr<DrawTarget> mUnlitTarget;

    // All these have 2 elements. Index 0 corresponds to the plane and index 1
    // corresponds to the sphere.
    std::vector<std::shared_ptr<Visual>> mVisuals;
    std::vector<std::shared_ptr<SMSceneEffect>> mSceneEffects;
    std::vector<std::shared_ptr<SMShadowEffect>> mShadowEffects;
    std::vector<std::shared_ptr<SMUnlitEffect>> mUnlitEffects;

    std::shared_ptr<SMBlurEffect> mBlurHEffect;
    std::shared_ptr<SMBlurEffect> mBlurVEffect;
    std::shared_ptr<DrawTarget> mBlurHTarget;
    std::shared_ptr<DrawTarget> mBlurVTarget;
};
