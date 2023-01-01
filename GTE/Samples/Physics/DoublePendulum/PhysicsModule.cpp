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
    mass1(0.0f),
    mass2(0.0f),
    length1(0.0f),
    length2(0.0f),
    jointX(0.0f),
    jointY(0.0f),
    mState{ 0.0f, 0.0f, 0.0f, 0.0f },
    mTime(0.0f),
    mAux{ 0.0f, 0.0f, 0.0f, 0.0f }
{
}

void PhysicsModule::Initialize(float time, float deltaTime, float theta1, float theta2,
    float theta1Dot, float theta2Dot)
{
    mTime = time;

    // State variables.
    mState[0] = theta1;
    mState[1] = theta1Dot;
    mState[2] = theta2;
    mState[3] = theta2Dot;

    // Auxiliary variables.
    mAux[0] = gravity;
    mAux[1] = length1;
    mAux[2] = length2;
    mAux[3] = mass2 / (mass1 + mass2);

    // RK4 differential equation solver.
    std::function<Vector4<float>(float, Vector4<float> const&)> odeFunction
        =
        [this](float, Vector4<float> const& input) -> Vector4<float>
        {
            float angleD = input[0] - input[2];
            float csD = std::cos(angleD);
            float snD = std::sin(angleD);
            float invDet = 1.0f / (mAux[1] * mAux[2] * (1.0f - mAux[3] * csD * csD));
            float sn0 = std::sin(input[0]);
            float sn2 = std::sin(input[2]);
            float b1 = -mAux[0] * sn0 - mAux[3] * mAux[2] * snD*input[3] * input[3];
            float b2 = -mAux[0] * sn2 + mAux[1] * snD*input[1] * input[1];
            float theta1DotFunction = (b1 - mAux[3] * csD * b2) * mAux[2] * invDet;
            float theta2DotFunction = (b2 - csD * b1) * mAux[1] * invDet;

            // (theta1, dot(theta1), theta2, dot(theta2)
            return Vector4<float>{ input[1], theta1DotFunction, input[3], theta2DotFunction };
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

void PhysicsModule::GetPositions(float& x1, float& y1, float& x2, float& y2) const
{
    x1 = jointX + length1 * std::sin(mState[0]);
    y1 = jointY - length1 * std::cos(mState[0]);
    x2 = x1 + length2 * std::sin(mState[2]);
    y2 = y1 - length2 * std::cos(mState[2]);
}

void PhysicsModule::Update ()
{
    // Apply a single step of the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
