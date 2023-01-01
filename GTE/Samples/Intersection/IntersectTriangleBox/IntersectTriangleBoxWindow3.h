// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/ConstantColorEffect.h>
#include <Mathematics/IntrTriangle3OrientedBox3.h>
using namespace gte;

//#define USE_TIQUERY_OVERRIDE

class IntersectTriangleBoxWindow3 : public Window3
{
public:
    IntersectTriangleBoxWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void Translate(int32_t direction, float delta);
    void Rotate(int32_t direction, float delta);
    void DoIntersectionQuery();

    // The red effect indicates the box and triangle intersect.  The blue
    // effect indicates the box and triangle do not intersect.
    std::shared_ptr<Visual> mBoxMesh;
    std::shared_ptr<ConstantColorEffect> mRedEffect, mBlueEffect;

    // The green effect is for that part of the triangle (if any) outside
    // the box.  The gray effect is for that part of the triangle (if any)
    // inside the box.
    std::shared_ptr<Visual> mOutsideTriangleMesh, mInsideTriangleMesh;
    std::shared_ptr<ConstantColorEffect> mGreenEffect, mGrayEffect;

    // The polygons are double sided.
    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // All color effects have some transparency.
    std::shared_ptr<BlendState> mBlendState;

    // The world objects and intersection queries.
    OrientedBox3<float> mBox;
    Triangle3<float> mTriangle;
    FIQuery<float, Triangle3<float>, OrientedBox3<float>> mFIQuery;

#if defined(USE_TIQUERY_OVERRIDE)
    TIQuery<float, Triangle3<float>, OrientedBox3<float>> mTIQuery;
#endif
};
