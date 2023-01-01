// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/IntrSphere3Cone3.h>
using namespace gte;

class IntersectSphereConeWindow3 : public Window3
{
public:
    IntersectSphereConeWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void Translate(int32_t direction, float delta);
    void Rotate(int32_t direction, float delta);
    void TestIntersection();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<Visual> mConeMesh, mDiskMinMesh, mDiskMaxMesh, mSphereMesh;
    std::shared_ptr<ConstantColorEffect> mBlueEffect, mCyanEffect, mRedEffect;
    std::shared_ptr<ConstantColorEffect> mGreenEffect[2], mYellowEffect[2];
    float mAlpha;

    Sphere3<float> mSphere;
    Cone3<float> mCone;
    TIQuery<float, Sphere3<float>, Cone3<float>> mQuery;
};
