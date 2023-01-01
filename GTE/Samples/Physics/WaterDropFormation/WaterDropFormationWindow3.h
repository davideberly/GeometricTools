// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.12

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/NURBSCurve.h>
#include <Mathematics/Timer.h>
#include "RevolutionSurface.h"
using namespace gte;

class WaterDropFormationWindow3 : public Window3
{
public:
    WaterDropFormationWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateCeilingAndWall();
    void CreateWaterRoot();

    // water surface
    void CreateSpline0AndTargets();
    void CreateConfiguration0();

    // split into water surface and water drop
    void CreateSpline1();
    void CreateCircle1();
    void CreateConfiguration1();

    void DoPhysical1();
    void DoPhysical2();
    void DoPhysical3();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<BlendState> mBlendState;
    VertexFormat mVFormat;
    std::shared_ptr<Node> mScene, mWaterRoot;
    std::shared_ptr<Visual> mCeiling, mWall, mWaterSurface, mWaterDrop;
    std::shared_ptr<Texture2> mWaterTexture;
    std::shared_ptr<RevolutionSurface> mWaterSurfaceRevolution;
    std::shared_ptr<RevolutionSurface> mWaterDropRevolution;

    // Water curves and simulation parameters.
    std::shared_ptr<NURBSCurve<2, float>> mSpline, mCircle;
    std::vector<Vector2<float>> mTargets;
    float mSimTime, mSimDelta;

    // Support for clamping the frame rate.
    Timer mMotionTimer;
    double mLastMotionTime, mCurrMotionTime;
};
