// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "RoughPlaneParticle1Window2.h"

//#define SINGLE_STEP

RoughPlaneParticle1Window2::RoughPlaneParticle1Window2(Parameters& parameters)
    :
    Window2(parameters)
{
    // Set up the physics module.
    mModule.gravity = 10.0;
    mModule.mass = 10.0;
    mModule.friction = 1.0;
    mModule.angle = 0.125 * GTE_C_PI;

    // Initialize the differential equations.
    double time = 0.0;
    double deltaTime = 1.0 / 60.0;
    double x = 0.0;
    double w = 0.0;
    double xDer = 10.0;
    double wDer = 40.0;
    mModule.Initialize(time, deltaTime, x, w, xDer, wDer);

    // Initialize the coefficients for the viscous friction solution.
    mR = mModule.friction / mModule.mass;
    mA0 = -xDer / mR;
    mA1 = x - mA0;
    mB1 = -mModule.gravity * std::sin(mModule.angle) / mR;
    mB2 = (wDer + mR * w - mB1) / mR;
    mB0 = w - mB2;

    // Save path of motion.
    mVFPositions.push_back(GetVFPosition(time));
    mSFPositions.push_back(Vector2<double>{ x, w });

    mDoFlip = true;
    OnDisplay();
}

void RoughPlaneParticle1Window2::OnIdle()
{
#ifndef SINGLE_STEP
    if (mContinueSolving)
    {
        mModule.Update();
        if (mModule.GetX() > 0.0 && mModule.GetW() <= 0.0)
        {
            mContinueSolving = false;
            return;
        }

        mVFPositions.push_back(GetVFPosition(mModule.GetTime()));
        mSFPositions.push_back(Vector2<double>{ mModule.GetX(), mModule.GetW() });
        OnDisplay();
    }
#endif
}

void RoughPlaneParticle1Window2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFF808080;
    uint32_t const blue = 0xFF800000;
    uint32_t const lightBlue = 0xFFFF0000;
    int32_t x0{}, w0{}, x1{}, w1{}, i{};
    Vector2<double> position{};

    double const xScale = 1.25;
    double const wScale = 0.75;
    int32_t const wOffset = 96;

    // Draw viscous friction path of motion.
    int32_t const numVFPositions = static_cast<int32_t>(mVFPositions.size());
    position = mVFPositions[0];
    x0 = static_cast<int32_t>(xScale * position[0] + 0.5);
    w0 = static_cast<int32_t>(wScale * position[1] + 0.5) + wOffset;
    x1 = x0;
    w1 = w0;
    for (i = 1; i < numVFPositions; ++i)
    {
        position = mVFPositions[i];
        x1 = static_cast<int32_t>(xScale * position[0] + 0.5);
        w1 = static_cast<int32_t>(wScale * position[1] + 0.5) + wOffset;
        DrawLine(x0, w0, x1, w1, lightBlue);
        x0 = x1;
        w0 = w1;
    }

    // Draw the mass.
    DrawThickPixel(x1, w1, 2, blue);

    // Draw static friction path of motion.
    int32_t const numSFPositions = static_cast<int32_t>(mSFPositions.size());
    position = mSFPositions[0];
    x0 = static_cast<int32_t>(xScale * position[0] + 0.5);
    w0 = static_cast<int32_t>(wScale * position[1] + 0.5) + wOffset;
    x1 = x0;
    w1 = w0;
    for (i = 1; i < numSFPositions; ++i)
    {
        position = mSFPositions[i];
        x1 = static_cast<int32_t>(xScale * position[0] + 0.5);
        w1 = static_cast<int32_t>(wScale * position[1] + 0.5) + wOffset;
        DrawLine(x0, w0, x1, w1, gray);
        x0 = x1;
        w0 = w1;
    }

    // Draw the mass.
    DrawThickPixel(x1, w1, 2, black);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool RoughPlaneParticle1Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
#ifdef SINGLE_STEP
    switch (key)
    {
    case 'g':
    case 'G':
        if (mContinueSolving)
        {
            mModule.Update();
            if (mModule.GetX() > 0.0 && mModule.GetW() <= 0.0)
            {
                mContinueSolving = false;
                return true;
            }
            mVFPositions.push_back(GetVFPosition(mModule.GetTime()));
            mSFPositions.push_back(Vector2d(mModule.GetX(), mModule.GetW()));
            OnDisplay();
        }
        return true;
    }
#endif

    return Window2::OnCharPress(key, x, y);
}

Vector2<double> RoughPlaneParticle1Window2::GetVFPosition(double time)
{
    Vector2<double> position{};

    double expValue = std::exp(-mR * time);
    position[0] = mA0 * expValue + mA1;
    position[1] = mB0 * expValue + mB1 * time + mB2;

    return position;
}
