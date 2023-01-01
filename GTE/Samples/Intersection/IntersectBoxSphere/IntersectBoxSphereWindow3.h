// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/IntrOrientedBox3Sphere3.h>
using namespace gte;

class IntersectBoxSphereWindow3 : public Window3
{
public:
    IntersectBoxSphereWindow3(Parameters& parameters);

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
    std::shared_ptr<Visual> mSphereMesh, mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    Sphere3<float> mSphere;
    OrientedBox3<float> mBox;
    TIQuery<float, OrientedBox3<float>, Sphere3<float>> mQuery;
};
