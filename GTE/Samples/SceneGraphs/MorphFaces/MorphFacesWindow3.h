// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Graphics/PointLightEffect.h>
#include "CubicInterpolator.h"
#include <chrono>
using namespace gte;

class MorphFacesWindow3 : public Window3
{
public:
    MorphFacesWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateMorphResult();
    void LoadTarget(int32_t i, std::string const& targetName);
    void UpdateMorph(float time);

    std::shared_ptr<Node> mScene, mMorphResult;
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Lighting> mLighting;
    std::shared_ptr<LightCameraGeometry> mLightGeometry;
    std::array<std::shared_ptr<PointLightEffect>, 4> mPLEffects;
    std::vector<std::shared_ptr<Visual>> mVisuals;

    static int32_t constexpr NUM_TARGETS = 12;

    struct InVertex
    {
        Vector3<float> position, normal;
    };

    struct OutVertex
    {
        Vector3<float> position, normal;
        Vector2<float> tcoord;
    };

    int32_t mNumVertices;
    std::array<std::vector<InVertex>, NUM_TARGETS> mVertices;
    std::array<std::shared_ptr<CubicInterpolator<1, float>>, NUM_TARGETS> mWeightInterpolator;
    std::shared_ptr<CubicInterpolator<3, float>> mColorInterpolator;
    Vector4<float> mLightWorldPosition;
    Timer mAnimTimer;
    double mAnimStartTime;
};
