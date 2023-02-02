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

//#define FOUCAULT_PENDULUM_SINGLE_STEP

class FoucaultPendulumWindow3 : public Window3
{
public:
    FoucaultPendulumWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreatePath();
    void CreatePendulum();
    void PhysicsTick();
    void GraphicsTick();

    // The vertex format for the path the pendulum tip follows.
    struct VertexPC
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    // The vertex format for the meshes of the pendulum.
    struct VertexPT
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mPendulum;
    std::shared_ptr<Visual> mPath;
    uint32_t mNextPoint;
    float mColorDiff;
    std::vector<std::shared_ptr<Visual>> mVisuals;

    // The physics system for the Foucault pendulum.
    PhysicsModule mModule;
};
