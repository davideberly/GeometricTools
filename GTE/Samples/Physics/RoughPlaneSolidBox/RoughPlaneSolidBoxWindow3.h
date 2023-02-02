// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/Timer.h>
#include "PhysicsModule.h"
using namespace gte;

// This is an implementation of an algorithm in Section 6 of
// https://www.geometrictools.com/Documentation/RoughPlaneAnalysis.pdf

class RoughPlaneSolidBoxWindow3 : public Window3
{
public:
    RoughPlaneSolidBoxWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateGround();
    void CreateRamp();
    void CreateBox();

    void InitializeModule();
    void MoveBox();
    void PhysicsTick();
    void GraphicsTick();

    struct VertexPT
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    struct VertexPC
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mBox;
    std::shared_ptr<Visual> mGround, mRamp;
    std::array<std::shared_ptr<Visual>, 6> mBoxFace;

    // The physics system.
    PhysicsModule mModule;
    bool mDoUpdate;
    Timer mPhysicsTimer;
    double mLastPhysicsTime, mCurrPhysicsTime;
};
