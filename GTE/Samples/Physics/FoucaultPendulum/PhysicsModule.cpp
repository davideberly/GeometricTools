// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    angularSpeed(0.0f),
    latitude(0.0f),
    gDivL(0.0f),
    mTime(0.0f),
    mState{ 0.0f, 0.0f, 0.0f, 0.0f },
    mAux{ 0.0f, 0.0f, 0.0f }
{
}

void PhysicsModule::Initialize(float time, float deltaTime, float theta,
    float phi, float thetaDot, float phiDot)
{
    mTime = time;

    // State variables.
    mState[0] = theta;
    mState[1] = thetaDot;
    mState[2] = phi;
    mState[3] = phiDot;

    // Auxiliary variables.
    mAux[0] = angularSpeed * std::sin(latitude);
    mAux[1] = angularSpeed * std::cos(latitude);
    mAux[2] = gDivL;

    // RK4 differential equation solver.
    std::function<Vector4<float>(float, Vector4<float> const&)> odeFunction
        =
        [this](float, Vector4<float> const& input) -> Vector4<float>
        {
            float sinTheta = std::sin(input[0]);
            float sinPhi = std::sin(input[2]);
            float cosPhi = std::cos(input[2]);

            // This function has a removable discontinuity at phi = 0.  When
            // sin(phi) is nearly zero, switch to the function that is
            // defined at phi = 0.
            float const epsilon = 1e-06f;
            float theta1DotFunction;
            if (std::fabs(sinPhi) < epsilon)
            {
                theta1DotFunction = (2.0f / 3.0f) * mAux[1] * input[3] * sinTheta;
            }
            else
            {
                theta1DotFunction = -2.0f * input[3] * (-mAux[1] * sinTheta +
                    cosPhi * (input[1] + mAux[0]) / sinPhi);
            }

            float theta2DotFunction = sinPhi * (input[1] * input[1] * cosPhi +
                2.0f * input[1] * (mAux[1] * sinTheta*sinPhi - mAux[0] * cosPhi) - mAux[2]);

            // (theta, dot(theta), phi, dot(phi)
            return Vector4<float>{ input[1], theta1DotFunction, input[3], theta2DotFunction };
        };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

Matrix4x4<float> PhysicsModule::GetOrientation() const
{
    float cosTheta = std::cos(mState[0]);
    float sinTheta = std::sin(mState[0]);
    float cosPhi = std::cos(mState[2]);
    float sinPhi = std::sin(mState[2]);
    float oneMinusCosPhi = 1.0f - cosPhi;

    Matrix4x4<float> rot;
#if defined(GTE_USE_MAT_VEC)
    rot(0, 0) = 1.0f - oneMinusCosPhi * cosTheta * cosTheta;
    rot(0, 1) = -oneMinusCosPhi * sinTheta * cosTheta;
    rot(0, 2) = -sinPhi*cosTheta;
    rot(0, 3) = 0.0f;
    rot(1, 0) = rot(0, 1);
    rot(1, 1) = 1.0f - oneMinusCosPhi * sinTheta * sinTheta;
    rot(1, 2) = -sinPhi * sinTheta;
    rot(1, 3) = 0.0f;
    rot(2, 0) = -rot(0, 2);
    rot(2, 1) = -rot(1, 2);
    rot(2, 2) = cosPhi;
    rot(2, 3) = 0.0f;
    rot(3, 0) = 0.0f;
    rot(3, 1) = 0.0f;
    rot(3, 2) = 0.0f;
    rot(3, 3) = 1.0f;
#else
    rot(0, 0) = 1.0f - oneMinusCosPhi * cosTheta * cosTheta;
    rot(1, 0) = -oneMinusCosPhi * sinTheta * cosTheta;
    rot(2, 0) = -sinPhi*cosTheta;
    rot(3, 0) = 0.0f;
    rot(0, 1) = rot(1, 0);
    rot(1, 1) = 1.0f - oneMinusCosPhi * sinTheta * sinTheta;
    rot(2, 1) = -sinPhi * sinTheta;
    rot(3, 1) = 0.0f;
    rot(0, 2) = -rot(2, 0);
    rot(1, 2) = -rot(2, 1);
    rot(2, 2) = cosPhi;
    rot(3, 2) = 0.0f;
    rot(0, 3) = 0.0f;
    rot(1, 3) = 0.0f;
    rot(2, 3) = 0.0f;
    rot(3, 3) = 1.0f;
#endif
    return rot;
}

void PhysicsModule::Update()
{
    // Apply a single step to the ODE solver.
    if (mSolver)
    {
        mSolver->Update(mTime, mState, mTime, mState);
    }
}
