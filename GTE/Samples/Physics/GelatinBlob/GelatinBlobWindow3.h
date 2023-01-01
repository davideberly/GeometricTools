// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include "PhysicsModule.h"
using namespace gte;

//#define GELATIN_BLOB_SINGLE_STEP

class GelatinBlobWindow3 : public Window3
{
public:
    GelatinBlobWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateIcosahedron();
    void CreateSprings();
    void CreateSegments();
    void PhysicsTick();
    void GraphicsTick();

    // The vertex layout for the icosahedron.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<BlendState> mBlendState;
    std::shared_ptr<DepthStencilState> mDepthReadNoWriteState;
    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Node> mScene, mSegmentRoot;
    std::shared_ptr<Visual> mIcosahedron;
    std::vector<std::shared_ptr<Visual>> mSegments;

    // The physics system.
    std::unique_ptr<PhysicsModule> mModule;
    Timer mMotionTimer;
};
