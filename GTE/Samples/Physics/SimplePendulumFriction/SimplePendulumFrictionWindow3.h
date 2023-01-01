// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include "PhysicsModule.h"
using namespace gte;

class SimplePendulumFrictionWindow3 : public Window3
{
public:
    SimplePendulumFrictionWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreatePendulum();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mPendulum;
    std::vector<std::shared_ptr<Visual>> mVisuals;

    // The physics system for the simple pendulum with friction.
    PhysicsModule mModule;
    int32_t mMotionType;
};
