// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Mathematics/BSplineCurve.h>
#include <Mathematics/TubeMesh.h>
#include "PhysicsModule.h"
using namespace gte;

//#define MASS_PULLEY_SPRING_SYSTEM_SINGLE_STEP

class MassPulleySpringSystemWindow3 : public Window3
{
public:
    MassPulleySpringSystemWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void InitializeModule();
    void CreateScene();
    void CreateFloor();
    void CreateCable();
    std::shared_ptr<Visual> CreateMass(float radius);
    void CreatePulley();
    void CreateSpring();
    void CreateHelix();
    void UpdatePulley();
    void UpdateCable();
    void UpdateHelix();
    void PhysicsTick();
    void GraphicsTick();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // Root of scene and floor mesh.
    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mFloor;
    std::shared_ptr<Node> mScene;
    std::vector<std::shared_ptr<Visual>> mVisuals;

    // Assembly to parent the cable root and pulley root.
    std::shared_ptr<Node> mAssembly;

    // Cable modeled as a tube surface, masses attached to ends.
    std::shared_ptr<Node> mCableRoot;
    std::shared_ptr<BSplineCurve<3, float>> mCableSpline;
    std::unique_ptr<TubeMesh<float>> mCableSurface;
    std::shared_ptr<Visual> mCable;
    std::shared_ptr<Visual> mMass1, mMass2;

    // Node to parent the pulley and spring.
    std::shared_ptr<Node> mPulleyRoot;

    // Pulley modeled as a disk with thickness.
    std::shared_ptr<Node> mPulley;
    std::shared_ptr<Visual> mPlate0, mPlate1, mCylinder;

    // Spring modeled as a tube surface in the shape of a helix, then attached
    // to a U-bracket to hold the pulley disk.
    std::shared_ptr<Node> mSpring;
    std::shared_ptr<Visual> mSide0, mSide1, mTop;
    std::shared_ptr<BSplineCurve<3, float>> mHelixSpline;
    std::unique_ptr<TubeMesh<float>> mHelixSurface;
    std::shared_ptr<Visual> mHelix;

    // The physics system.
    PhysicsModule mModule;

    // Support for clamping the frame rate.
    Timer mMotionTimer;
    double mLastUpdateTime;
};
