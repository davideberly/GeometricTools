// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/IntrAlignedBox3Cone3.h>
#include <Mathematics/IntrOrientedBox3Cone3.h>
using namespace gte;

#define USE_ORIENTED_BOX

class IntersectBoxConeWindow3 : public Window3
{
public:
    IntersectBoxConeWindow3(Parameters& parameters);

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
    std::shared_ptr<Visual> mConeH0Mesh, mConeH4Mesh;
    std::shared_ptr<Visual> mDiskMaxMesh, mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;
    Cone<3, float> mCone;
#if defined(USE_ORIENTED_BOX)
    TIOrientedBox3Cone3<float> mQuery;
    OrientedBox<3, float> mBox;
#else
    TIAlignedBox3Cone3<float> mQuery;
    AlignedBox<3, float> mBox;
#endif
};
