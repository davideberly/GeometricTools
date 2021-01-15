// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/MassSpringVolume.h>
#include <Mathematics/Vector3.h>
using namespace gte;

class PhysicsModule : public MassSpringVolume<3, float>
{
public:
    // Construction.
    PhysicsModule(int numSlices, int numRows, int numCols, float step, float viscosity);

    // External acceleration is due to viscous forces.
    virtual Vector3<float> ExternalAcceleration(int i, float time,
        std::vector<Vector3<float>> const& positions,
        std::vector<Vector3<float>> const& velocities);

protected:
    float mViscosity;
};
