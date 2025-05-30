// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Mathematics/MassSpringArbitrary.h>
#include <Mathematics/Vector3.h>
using namespace gte;

class PhysicsModule : public MassSpringArbitrary<3, float>
{
public:
    // Construction.
    PhysicsModule(int32_t numParticles, int32_t numSteps, float step, float viscosity);

    // External acceleration is due to viscous forces.
    virtual Vector3<float> ExternalAcceleration(int32_t i, float time,
        std::vector<Vector3<float>> const& positions,
        std::vector<Vector3<float>> const& velocities);

protected:
    float mViscosity;
};

