// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.16

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/RigidBody.h>
#include <Mathematics/Hyperplane.h>
#include <Mathematics/Hypersphere.h>
#include <Mathematics/Timer.h>
#include "PhysicsModule.h"
using namespace gte;

// The physics module is implemented based on the algorithms described in my
// book "Game Physics, 2nd edition" by CRC Press (April 2020). The relevant
// material is in Section 6.6 Acceleration-Based Constrained Motion. The
// masses of the spheres are NOT proportional to the volumes of the spheres,
// which can make portions of the simulation appear to be nonrealistic. Feel
// free to modify Initial.txt to use masses that are proportional to the
// volumes.

class BouncingSpheresWindow3 : public Window3
{
public:
    BouncingSpheresWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    struct VertexPT
    {
        VertexPT()
            :
            position(Vector3<float>::Zero()),
            tcoord(Vector2<float>::Zero())
        {
        }

        Vector3<float> position;
        Vector2<float> tcoord;
    };

    struct VertexPC
    {
        VertexPC()
            :
            position(Vector3<float>::Zero()),
            color(Vector4<float>::Zero())
        {
        }

        Vector3<float> position;
        Vector4<float> color;
    };

    bool SetEnvironment();
    void CreateScene();
    void CreatePhysicsObjects();
    void CreateGraphicsObjects();
    void CreateWall(size_t index, VertexFormat const& vformat,
        Vector3<float> const& pos0, Vector3<float> const& pos1,
        Vector3<float> const& pos2, Vector3<float> const& pos3,
        Vector4<float> const& color);

    void PhysicsTick();
    void GraphicsTick();

    enum { NUM_SPHERES = 16 };
    std::unique_ptr<PhysicsModule> mModule;

    std::shared_ptr<RasterizerState> mNoCullState;
    std::shared_ptr<RasterizerState> mNoCullWireState;
    std::shared_ptr<Node> mScene;
    std::array<std::shared_ptr<Visual>, 4> mPlaneMesh;
    std::array<std::shared_ptr<Visual>, NUM_SPHERES> mSphereMesh;

    Timer mPhysicsTimer;
    double mLastPhysicsTime, mCurrPhysicsTime;
};
