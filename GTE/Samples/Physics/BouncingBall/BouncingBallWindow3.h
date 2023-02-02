// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.03.07

#pragma once

#include <Applications/Window3.h>
#include <Graphics/PlanarReflectionEffect.h>
#include "DeformableBall.h"
using namespace gte;

//#define BOUNCING_BALL_SINGLE_STEP

class BouncingBallWindow3 : public Window3
{
public:
    BouncingBallWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateBall();
    void CreateFloor();
    void CreateWall();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // Representation of body
    std::unique_ptr<DeformableBall> mBall;

    // Simulated clock.
    float mSimTime, mSimDelta;

    // The scene graph.
    std::shared_ptr<Node> mScene, mBallNode;
    std::shared_ptr<Visual> mFloor, mWall;
    std::shared_ptr<RasterizerState> mWireState;
    std::unique_ptr<PlanarReflectionEffect> mPlanarReflectionEffect;
};
