// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0),
    mass(0.0),
    friction(0.0),
    angle(0.0),
    mTime(0.0),
    mDeltaTime(0.0),
    mState{ 0.0, 0.0, 0.0, 0.0 },
    mAux{ 0.0, 0.0 },
    mSolver{}
{
}

void PhysicsModule::Initialize(double time, double deltaTime, double x,
    double w, double xDer, double wDer)
{
    mTime = time;
    mDeltaTime = deltaTime;

    // state variables
    mState[0] = x;
    mState[1] = xDer;
    mState[2] = w;
    mState[3] = wDer;

    // auxiliary variables
    mAux[0] = friction / mass;
    mAux[1] = gravity * std::sin(angle);

    // RK4 differential equation solver.
    std::function<Vector4<double>(double, Vector4<double> const&)> odeFunction =
    [this](double, Vector4<double> const& input)
    {
        double vLen = std::sqrt(input[1] * input[1] + input[3] * input[3]);
        double xDerFunction, wDerFunction;
        if (vLen > 0.0)
        {
            double temp = -mAux[0] / vLen;
            xDerFunction = temp * input[1];
            wDerFunction = temp * input[3] - mAux[1];
        }
        else
        {
            // Velocity is effectively zero, so frictional force is zero.
            xDerFunction = 0.0;
            wDerFunction = -mAux[1];
        }

        Vector4<double> output
        {
            // x function
            input[1],

            // dot(x) function
            xDerFunction,

            // w function
            input[3],

            // dot(w) function
            wDerFunction
        };

        return output;
    };

    mSolver = std::make_unique<Solver>(mDeltaTime, odeFunction);
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    mSolver->Update(mTime, mState, mTime, mState);
}
