// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.21

#pragma once

#include <Applications/Window3.h>
#include "RTSphereTriangle.h"
using namespace gte;

class CollisionsMovingSphereTriangleWindow3 : public Window3
{
public:
    CollisionsMovingSphereTriangleWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

    // Enhance the trackball by allowing it to rotate a Visual object
    // in the scene about that object's center.
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int32_t x, int32_t y, uint32_t modifiers) override;

    virtual bool OnMouseMotion(MouseButton button, int32_t x, int32_t y,
        uint32_t modifiers) override;

private:
    void CreateScene();
    void Update();

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mSphereMesh;
    std::shared_ptr<Visual> mTriangleMesh;
    std::shared_ptr<Visual> mContactMesh;
    std::shared_ptr<Visual> mCenters;
    std::vector<std::shared_ptr<Visual>> mVisuals;
    std::shared_ptr<Spatial> mMotionObject;

    RTSphereTriangle::Sphere mSphere;
    RTSphereTriangle::Triangle mTriangle;
    Vector3<float> mSphereVelocity, mTriangleVelocity;
    float mSimulationTime, mSimulationDeltaTime;
    float mContactTime;
    Vector3<float> mContactPoint;
    std::array<Vector3<float>, 3> mMTri;
    bool mUseInitialCenter;
};
