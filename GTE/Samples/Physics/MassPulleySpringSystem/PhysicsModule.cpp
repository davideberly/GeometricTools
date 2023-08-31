// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#include <Mathematics/Constants.h>
#include "PhysicsModule.h"
#include <cmath>

void PhysicsModule::Initialize(float time, float deltaTime, float y1, float dy1, float dy3)
{
    mTime = time;
    mDeltaTime = deltaTime;

    // Derived parameters.
    mAlpha = mass1 + mass2 + mass3;
    mBeta = mass1 - mass2;
    mGamma = mass1 + mass2 + inertia / (radius * radius);
    float tmp = springConstant * mGamma / (mAlpha * mGamma - mBeta * mBeta);
    mOmega = std::sqrt(tmp);
    float omegaSqr = mOmega * mOmega;
    mGDivOmegaSqr = gravity / omegaSqr;
    mDelta = mBeta * omegaSqr / mGamma;

    // The initial conditions of the system.
    mY1 = y1;
    mDY1 = dy1;
    mY2 = wireLength - radius * static_cast<float>(GTE_C_PI) - mY1;
    mDY2 = -mDY1;
    mY3 = springLength;
    mDY3 = dy3;

    // The solution parameters.
    mLPlusGDivOmegaSqr = springLength + mGDivOmegaSqr;
    mK1 = mDY3 / mOmega;
    mK2 = mY3 - mLPlusGDivOmegaSqr;
    mTCoeff = mDY1 + mDelta * mK1 / mOmega;
    mDeltaDivOmegaSqr = mDelta / omegaSqr;
    mTSqrCoeff = 0.5f * gravity * mDeltaDivOmegaSqr;

    // The initial values of the solution.
    mY1Curr = mY1;
    mY2Curr = mY2;
    mY3Curr = mY3;
}

void PhysicsModule::Update()
{
    if (mY1Curr <= mY3Curr || mY2Curr <= mY3Curr)
    {
        // One of the masses has "lodged" in the pulley.  No more motion is
        // allowed.
        return;
    }

    if (mY1Curr >= 255.0f || mY2Curr >= 255.0f)
    {
        // One of the masses has reached the floor.  No more motion is
        // allowed.
        return;
    }

    mTime += mDeltaTime;

    float scaledTime = mOmega * mTime;
    float sn = std::sin(scaledTime);
    float cs = std::cos(scaledTime);
    float combo = mK1 * sn + mK2 * cs;

    mY3Curr = combo + mLPlusGDivOmegaSqr;
    mY1Curr = mY1 + mTime * (mTCoeff + mTSqrCoeff * mTime) - mDeltaDivOmegaSqr * combo;
    mY2Curr = wireLength - radius * static_cast<float>(GTE_C_PI) - mY1Curr;
}
