// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Vector4.h>
#include <Mathematics/OdeRungeKutta4.h>
#include <memory>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the differential equation solver.
    void Initialize(float time, float deltaTime, float theta, float thetaDot,
        float radius, float radiusDot);

    // Member access.
    float GetPeriod() const;

    // Apply a single step of the solver.
    void Update();

    // Access the current state.
    inline float GetTime() const
    {
        return mTime;
    }

    inline float GetTheta() const
    {
        return mState[0];
    }

    inline float GetThetaDot() const
    {
        return mState[1];
    }

    inline float GetRadius() const
    {
        return mState[2];
    }

    inline float GetRadiusDot() const
    {
        return mState[3];
    }

    // physical constants
    float gravity;
    float mass;

private:
    // State and auxiliary variables.
    Vector4<float> mState;
    float mTime;
    std::array<float, 5> mAux;

    // Ellipse parameters.
    float mEccentricity, mRho, mMajorAxis, mMinorAxis;

    // Runge-Kutta 4th-order ODE solver.
    typedef OdeRungeKutta4<float, float> Solver;
    std::unique_ptr<Solver> mSolver;
};
