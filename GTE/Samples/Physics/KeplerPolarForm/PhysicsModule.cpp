// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#include <Mathematics/Constants.h>
#include "PhysicsModule.h"

PhysicsModule::PhysicsModule()
    :
    gravity(0.0f),
    mass(0.0f),
    mState{ 0.0f, 0.0f, 0.0f, 0.0f },
    mTime(0.0f),
    mAux{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    mEccentricity(0.0f),
    mRho(0.0f),
    mMajorAxis(0.0f),
    mMinorAxis(0.0f)
{
}

void PhysicsModule::Initialize (float time, float deltaTime, float theta, float thetaDot,
    float radius, float radiusDot)
{
    mTime = time;

    // State variables.
    mState[0] = theta;
    mState[1] = thetaDot;
    mState[2] = radius;
    mState[3] = radiusDot;

    // Compute c0 and c1 in the potential energy function V(theta).
    float gm = gravity * mass;
    float gm2 = gm * mass;
    float radiusSqr = radius * radius;
    float alpha = mass * radiusSqr * thetaDot;
    float g2m4da2 = gm2 * gm2 / (alpha * alpha);
    float v0 = -gm / radius;
    float dv0 = gm2 * radiusDot / alpha;
    float v0Plus = v0 + g2m4da2;
    float sinTheta0 = std::sin(theta);
    float cosTheta0 = std::cos(theta);
    float c0 = v0Plus * sinTheta0 + dv0 * cosTheta0;
    float c1 = v0Plus * cosTheta0 - dv0 * sinTheta0;

    // Auxiliary variables needed by function DTheta(...).
    mAux[0] = c0;
    mAux[1] = c1;
    mAux[2] = g2m4da2;
    mAux[3] = alpha /(gm * gm2);

    // Ellipse parameters.
    float gamma0 = radiusSqr * std::fabs(thetaDot);
    float tmp0 = radiusSqr * radius * thetaDot * thetaDot - gm;
    float tmp1 = radiusSqr * radiusDot * thetaDot;
    float gamma1 = std::sqrt(tmp0 * tmp0 + tmp1 * tmp1);
    mEccentricity = gamma1 / gm;
    mRho = gamma0 * gamma0 / gamma1;
    float tmp2 = 1.0f - mEccentricity * mEccentricity;  // > 0
    mMajorAxis = mRho * mEccentricity / tmp2;
    mMinorAxis = mMajorAxis * std::sqrt(tmp2);

    // RK4 differential equation solver.
    std::function<float(float, float const&)> odeFunction
        =
        [this](float, float const& input) -> float
    {
        float sn = std::sin(input);
        float cs = std::cos(input);
        float v = mAux[0] * sn + mAux[1] * cs - mAux[2];
        float thetaDotFunction = mAux[3] * v * v;
        return thetaDotFunction;
    };

    mSolver = std::make_unique<Solver>(deltaTime, odeFunction);
}

float PhysicsModule::GetPeriod() const
{
    float powValue = std::pow(mMajorAxis, 1.5f);
    float sqrtValue = std::sqrt(gravity * mass);
    return static_cast<float>(GTE_C_TWO_PI) * powValue / sqrtValue;
}

void PhysicsModule::Update()
{
    if (mSolver)
    {
        // Apply a single step of the ODE solver.
        mSolver->Update(mTime, mState[0], mTime, mState[0]);

        // Compute dot(theta) for application access.
        float sn = std::sin(mState[0]);
        float cs = std::cos(mState[0]);
        float v = mAux[0] * sn + mAux[1] * cs - mAux[2];
        mState[1] = mAux[3] * v * v;

        // Compute radius for application access.
        mState[2] = mEccentricity * mRho / (1.0f + mEccentricity * cs);

        // Compute dot(radius) for application access.
        mState[3] = mState[2] * mState[2] * mState[1] * sn / mRho;
    }
}
