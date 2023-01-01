// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/Timer.h>
#include "SphereColliders.h"
using namespace gte;

class CollisionsMovingSpheresWindow3 : public Window3
{
public:
    CollisionsMovingSpheresWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();
    void UpdateSpheres();

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<Visual> mMesh0, mMesh1;

    Sphere3<float> mSphere0, mSphere1, mBoundingSphere;
    Vector3<float> mVelocity0, mVelocity1;
    SphereColliders mColliders;
    float mSimulationTime, mSimulationDeltaTime;
};
