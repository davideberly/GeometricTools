// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/IntrTriangle3Cylinder3.h>
using namespace gte;

class IntersectTriangleCylinderWindow3 : public Window3
{
public:
    IntersectTriangleCylinderWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void Translate(int32_t direction, float delta);
    void Rotate(int32_t direction, float delta);
    void DoIntersectionQuery();

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Visual> mTriangleMesh;
    std::shared_ptr<Visual> mCylinderMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect;
    std::shared_ptr<ConstantColorEffect> mGreenEffect;
    std::shared_ptr<ConstantColorEffect> mBlueEffect;

    Triangle3<float> mTriangle;
    Cylinder3<float> mCylinder;
    TIQuery<float, Triangle3<float>, Cylinder3<float>> mQuery;

    size_t mMotionObject;
    Vector3<float> mTriangleCenter;
    std::array<Vector2<float>, 3> mTriangleCoord;
    Matrix3x3<float> mTriangleBasis;
    Matrix3x3<float> mCylinderBasis;
};
