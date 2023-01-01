// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Mathematics/BSplineSurface.h>
#include <Mathematics/RectanglePatchMesh.h>
#include "PhysicsModule.h"
using namespace gte;

class ClothWindow3 : public Window3
{
public:
    ClothWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateSprings();
    void CreateCloth();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mWireNoCullState;
    std::shared_ptr<Visual> mCloth;

    // The masses are located at the control points of a spline curve.  The
    // control points are connected by a mass-spring system.
    std::unique_ptr<PhysicsModule> mModule;
    std::shared_ptr<BSplineSurface<3, float>> mSpline;
    std::unique_ptr<RectanglePatchMesh<float>> mSurface;

    Timer mAnimTimer;
    double mAnimStartTime;
};
