// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "RoughPlaneParticle2Window2.h"

//#define SINGLE_STEP

RoughPlaneParticle2Window2::RoughPlaneParticle2Window2(Parameters& parameters)
    :
    Window2(parameters)
{
    mIteration = 0;
    mMaxIteration = 512;
    mSize = mXSize;

    // Set up the physics module.
    mModule.gravity = 10.0;
    mModule.mass1 = 10.0;
    mModule.mass2 = 20.0;
    mModule.friction1 = 1.0;
    mModule.friction2 = 1.0;

    // Initialize the differential equations.
    double time = 0.0;
    double deltaTime = 1.0 / 60.0;
    double x1 = 16.0;
    double y1 = 116.0;
    double x2 = 100.0;
    double y2 = 200.0;
    double xDot = 10.0;
    double yDot = -10.0;
    double thetaDot = 0.5;
    mModule.Initialize(time, deltaTime, x1, y1, x2, y2, xDot, yDot, thetaDot);

    mDoFlip = true;
    OnDisplay();
}

void RoughPlaneParticle2Window2::OnIdle()
{
#ifndef SINGLE_STEP
    if (mIteration < mMaxIteration)
    {
        mModule.Update();
        OnDisplay();
        ++mIteration;
    }
#endif
}

void RoughPlaneParticle2Window2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFF808080;
    uint32_t const blue = 0xFFFF0000;

    // Draw the rod.
    double dx1, dy1, dx2, dy2;
    mModule.Get(dx1, dy1, dx2, dy2);
    int32_t x1 = static_cast<int32_t>(dx1 + 0.5);
    int32_t y1 = static_cast<int32_t>(dy1 + 0.5);
    int32_t x2 = static_cast<int32_t>(dx2 + 0.5);
    int32_t y2 = static_cast<int32_t>(dy2 + 0.5);
    DrawLine(x1, y1, x2, y2, gray);

    // Draw the masses.
    DrawThickPixel(x1, y1, 2, black);
    DrawThickPixel(x2, y2, 2, black);

    // Draw the center of mass.
    int32_t x = static_cast<int32_t>(mModule.GetX() + 0.5);
    int32_t y = static_cast<int32_t>(mModule.GetY() + 0.5);
    DrawThickPixel(x, y, 2, blue);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool RoughPlaneParticle2Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
#ifdef SINGLE_STEP
    switch (key)
    {
    case 'g':
    case 'G':
        if (mIteration < mMaxIteration)
        {
            mModule.Update();
            OnDisplay();
            ++mIteration;
        }
        return true;
    }
#endif

    return Window2::OnCharPress(key, x, y);
}
