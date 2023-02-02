// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0f),
    mass(0.0f),
    mState{ 0.0f, 0.0f },
    mTime(0.0f),
    mAux(0.0f)
{
}

void PhysicsModule::Initialize(float time, float deltaTime, float q, float qDot)
{
    mTime = time;

    // State variables.
    mState[0] = q;
    mState[1] = qDot;

    // Auxiliary variable.
    mAux = gravity;

    // RK4 differential equation solver.
    std::function<Vector2<float>(float, Vector2<float> const&)> odeFunction
        =
        [this](float, Vector2<float> const& input) -> Vector2<float>
        {
            float qSqr = input[0] * input[0];
            float qDotSqr = input[1] * input[1];
            float numer = -3.0f * mAux * qSqr - 2.0f * input[0] * (2.0f + 9.0f * qSqr) * qDotSqr;
            float denom = 1.0f + qSqr * (4.0f + 9.0f * qSqr);
            float qDotFunction = numer / denom;

            // (q, dot(q))
            return Vector2<float>{ input[1], qDotFunction };
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

void PhysicsModule::Update()
{
    // Apply a single step of the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
