// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule (int32_t numSlices, int32_t numRows, int32_t numCols, float step, float viscosity)
    :
    MassSpringVolume<3, float>(numSlices, numRows, numCols, step),
    mViscosity(viscosity)
{
}

Vector3<float> PhysicsModule::ExternalAcceleration(int32_t i, float,
    std::vector<Vector3<float>> const&, std::vector<Vector3<float>> const& velocities)
{
    Vector3<float> acceleration = -mViscosity * velocities[i];
    return acceleration;
}

