// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/MassSpringVolume.h>
#include <Mathematics/Vector3.h>
using namespace gte;

class PhysicsModule : public MassSpringVolume<3, float>
{
public:
    // Construction.
    PhysicsModule(int32_t numSlices, int32_t numRows, int32_t numCols, float step, float viscosity);

    // External acceleration is due to viscous forces.
    virtual Vector3<float> ExternalAcceleration(int32_t i, float time,
        std::vector<Vector3<float>> const& positions,
        std::vector<Vector3<float>> const& velocities);

protected:
    float mViscosity;
};
