// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Matrix4x4.h>
#include <Mathematics/OdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float theta, float phi,
        float thetaDot, float phiDot);

    // The orientation of the pendulum.
    Matrix4x4<float> GetOrientation() const;

    // Apply a single step of the solver.
    void Update();

    // The pendulum parameters.
    float angularSpeed;  // w
    float latitude;  // lat
    float gDivL;  // g/L

private:
    // State and auxiliary variables.
    float mTime;
    Vector4<float> mState;
    std::array<float, 3> mAux;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector4<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};
