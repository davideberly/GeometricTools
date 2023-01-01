// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "RoughPlaneThinRod2Window2.h"

RoughPlaneThinRod2Window2::RoughPlaneThinRod2Window2(Parameters& parameters)
    :
    Window2(parameters)
{
    mSize = mXSize;

    // Same starting values as in RoughPlaneParticle2.
    double x1 = 16.0;
    double y1 = 116.0;
    double x2 = 100.0;
    double y2 = 200.0;
    double xDelta = x2 - x1;
    double yDelta = y2 - y1;

    // Set up the physics module.
    mModule.length = std::sqrt(xDelta * xDelta + yDelta * yDelta);
    mModule.muGravity = 5.0;  // mu * g = c / delta0 from RoughPlaneThinRod1

    // Initialize the differential equations.
    double time = 0.0;
    double deltaTime = 1.0 / 60.0;
    double x = 0.5 * (x1 + x2);
    double y = 0.5 * (y1 + y2);
    double theta = std::atan2(yDelta, xDelta);
    double xDer = 10.0;
    double yDer = -10.0;
    double thetaDer = 4.0;
    mModule.Initialize(time, deltaTime, x, y, theta, xDer, yDer, thetaDer);

    mLastPhysicsTime = mPhysicsTimer.GetSeconds();
    mCurrPhysicsTime = 0.0;

    mDoFlip = true;
    OnDisplay();
}

void RoughPlaneThinRod2Window2::OnIdle()
{
#ifndef SINGLE_STEP
    // Execute the physics system at the desired frames per second.
    mCurrPhysicsTime = mPhysicsTimer.GetSeconds();
    double deltaTime = mCurrPhysicsTime - mLastPhysicsTime;
    if (deltaTime >= mModule.GetDeltaTime())
    {
        mModule.Update();
        mLastPhysicsTime = mCurrPhysicsTime;
    }

    OnDisplay();
#endif
}

void RoughPlaneThinRod2Window2::OnDisplay()
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

bool RoughPlaneThinRod2Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
#ifdef SINGLE_STEP
    switch (key)
    {
    case 'g':
    case 'G':
        mModule.Update();
        OnDisplay();
        return true;
    }
#endif

    return Window2::OnCharPress(key, x, y);
}
