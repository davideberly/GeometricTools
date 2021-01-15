// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "PhysicsModule.h"
#include <Mathematics/Math.h>
#include <random>

PhysicsModule::PhysicsModule(int numRows, int numCols, float step,
    Vector3<float> const& gravity, Vector3<float> const& wind,
    float viscosity,float amplitude)
    :
    MassSpringSurface<3, float>(numRows, numCols, step),
    mGravity(gravity),
    mWind(wind),
    mDirection(UnitCross(gravity, wind)),
    mViscosity(viscosity),
    mAmplitude(amplitude),
    mPhases(mNumParticles)
{
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, static_cast<float>(GTE_C_PI));
    for (int row = 0; row < mNumRows; ++row)
    {
        for (int col = 0; col < mNumCols; ++col)
        {
            mPhases[GetIndex(row, col)] = rnd(mte);
        }
    }
}

Vector<3, float> PhysicsModule::ExternalAcceleration(int i, float time,
    std::vector<Vector<3, float>> const&,
    std::vector<Vector<3, float>> const& velocity)
{
    // Acceleration due to gravity, wind, and viscosity.
    Vector3<float> acceleration = mGravity + mWind - mViscosity * velocity[i];

    // Add a sinusoidal perturbation.
    float amplitude = mAmplitude * std::sin(2.0f * time + mPhases[i]);
    acceleration += amplitude * mDirection;
    return acceleration;
}
