// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "RoughPlaneFlatBoardWindow2.h"

RoughPlaneFlatBoardWindow2::RoughPlaneFlatBoardWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    mSize = mXSize;

    // Set up the physics module.
    mModule.muGravity = 5.0;
    mModule.xLocExt = 16.0;
    mModule.yLocExt = 8.0;

    // Initialize the differential equations.
    double time = 0.0;
    double deltaTime = 1.0 / 60.0;
    double x = 20.0;
    double y = 230.0;
    double theta = GTE_C_QUARTER_PI;
    double xDer = 30.0;
    double yDer = -30.0;
    double thetaDer = 4.0;
    mModule.Initialize(time, deltaTime, x, y, theta, xDer, yDer, thetaDer);

    mLastPhysicsTime = mPhysicsTimer.GetSeconds();
    mCurrPhysicsTime = 0.0;

    mDoFlip = true;
    OnDisplay();
}

void RoughPlaneFlatBoardWindow2::OnIdle()
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

void RoughPlaneFlatBoardWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const black = 0xFF000000;

    // Draw the board.
    double dx00, dy00, dx10, dy10, dx11, dy11, dx01, dy01;
    mModule.GetRectangle(dx00, dy00, dx10, dy10, dx11, dy11, dx01, dy01);
    int32_t x00 = static_cast<int32_t>(dx00 + 0.5);
    int32_t y00 = static_cast<int32_t>(dy00 + 0.5);
    int32_t x10 = static_cast<int32_t>(dx10 + 0.5);
    int32_t y10 = static_cast<int32_t>(dy10 + 0.5);
    int32_t x11 = static_cast<int32_t>(dx11 + 0.5);
    int32_t y11 = static_cast<int32_t>(dy11 + 0.5);
    int32_t x01 = static_cast<int32_t>(dx01 + 0.5);
    int32_t y01 = static_cast<int32_t>(dy01 + 0.5);
    DrawLine(x00, y00, x10, y10, black);
    DrawLine(x10, y10, x11, y11, black);
    DrawLine(x11, y11, x01, y01, black);
    DrawLine(x01, y01, x00, y00, black);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool RoughPlaneFlatBoardWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
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
