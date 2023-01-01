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

    // Initialize the differential equation solver.  The inputs theta, phi,
    // and psi determine the body coordinate axes Xi1, Xi2, and Xi3.  The
    // angular velocity inputs are the coefficients in the body coordinate
    // system.
    void Initialize(float time, float deltaTime, float theta, float phi,
        float psi, float angVel1, float angVel2, float angVel3);

    // Member access.
    inline float GetPhi() const
    {
        return mState[1];
    }

    // The body coordinate axes in world coordinates.
    Matrix4x4<float> GetBodyAxes() const;

    // Apply a single step of the solver.
    void Update();

    // The physical constants.
    float gravity;
    float mass;
    float length;
    float inertia1, inertia3;  // inertia2 = inertia1

private:
    // State and auxiliary variables.
    float mTime;
    Vector3<float> mState;
    std::array<float, 5> mAux;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, Vector3<float>> Solver;
    std::unique_ptr<Solver> mSolver;
};
