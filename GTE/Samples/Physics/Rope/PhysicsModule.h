// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/MassSpringCurve.h>
#include <Mathematics/Vector3.h>
#include <random>
using namespace gte;

class PhysicsModule : public MassSpringCurve<3, float>
{
public:
    // Construction.  Gravity is controlled by the input 'gravity'.
    // Mass-spring systems tend to exhibit stiffness in the sense of numerical
    // stability.  To remedy this problem, a small amount of viscous friction
    // is added to the external force, -viscosity*velocity, where 'viscosity'
    // is a small positive constant.  The initial wind force is specified by
    // the caller.  The application of wind can be toggled by 'enableWind'.
    // The member 'enableWindChange' allows the wind direction to change
    // randomly, but each new direction is nearby the old direction in order
    // to obtain some sense of continuity of direction.  The magnitude of the
    // wind force is constant, the length of the initial force.
    PhysicsModule(int32_t numParticles, float step, Vector3<float> const& gravity,
        Vector3<float> const& wind, float windChangeAmplitude,
        float viscosity);

    bool enableWind;
    bool enableWindChange;

    // External acceleration is due to forces of gravitation, wind, and
    // viscous friction.  The wind forces are randomly generated.
    virtual Vector<3, float> ExternalAcceleration(int32_t i, float time,
        std::vector<Vector<3, float>> const& position,
        std::vector<Vector<3, float>> const& velocity);

protected:
    Vector3<float> mGravity, mWind;
    float mWindChangeAmplitude, mViscosity;
    std::mt19937 mMte;
    std::uniform_real_distribution<float> mRnd;
};
