// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule(int numParticles, int numSprings, float step, float viscosity)
    :
    MassSpringArbitrary<3, float>(numParticles, numSprings, step),
    mViscosity(viscosity)
{
}

Vector3<float> PhysicsModule::ExternalAcceleration(int i, float,
    std::vector<Vector3<float>> const&, std::vector<Vector3<float>> const& velocities)
{
    Vector3<float> acceleration = -mViscosity * velocities[i];
    return acceleration;
}
