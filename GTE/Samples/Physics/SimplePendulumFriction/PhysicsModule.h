// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Vector2.h>
#include <Mathematics/OdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float theta, float thetaDot);

    // Apply a single step of the solver.
    void Update();

    // Access the current state.
    inline float GetTheta() const
    {
        return mState[0];
    }

    // pendulum parameters
    float cDivM;  // friction_coefficient / mass
    float gDivL;  // gravity_constant / pendulum_length

private:
    // State and auxiliary variables.
    float mTime;
    Vector2<float> mState;
    std::array<float, 2> mAux;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector2<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};
