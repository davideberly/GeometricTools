// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "DoublePendulumWindow2.h"

DoublePendulumWindow2::DoublePendulumWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(mXSize)
{
    // Set up the physics module.
    mModule.gravity = 10.0f;
    mModule.mass1 = 10.0f;
    mModule.mass2 = 20.0f;
    mModule.length1 = 100.0f;
    mModule.length2 = 100.0f;
    mModule.jointX = static_cast<float>(mSize / 2);
    mModule.jointY = static_cast<float>(mSize - 8);

    // Initialize the differential equations.
    mModule.Initialize(0.0f, 0.01f, static_cast<float>(0.125 * GTE_C_PI),
        static_cast<float>(0.25 * GTE_C_PI), 0.0f, 0.0f);

    // Use right-handed display coordinates.
    mDoFlip = true;

    OnDisplay();
}

void DoublePendulumWindow2::OnIdle()
{
    mModule.Update();
    OnDisplay();
}

void DoublePendulumWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    unsigned int const black = 0xFF000000, gray = 0xFF808080, blue = 0xFFFF0000;

    float x1, y1, x2, y2;
    mModule.GetPositions(x1, y1, x2, y2);
    int ix1 = static_cast<int>(std::lrint(x1));
    int iy1 = static_cast<int>(std::lrint(y1));
    int ix2 = static_cast<int>(std::lrint(x2));
    int iy2 = static_cast<int>(std::lrint(y2));

    // Draw the axes.
    DrawLine(mSize / 2, 0, mSize / 2, mSize - 1, gray);
    DrawLine(0, 0, mSize - 1, 0, gray);

    // The pendulum joint.
    int jx = static_cast<int>(std::lrint(mModule.jointX));
    int jy = static_cast<int>(std::lrint(mModule.jointY));

    // Draw the pendulum rods.
    DrawLine(jx, jy, ix1, iy1, blue);
    DrawLine(ix1, iy1, ix2, iy2, blue);

    // Draw the pendulum joint.
    DrawCircle(jx, jy, 2, black, true);

    // Draw the pendulum masses.
    DrawCircle(ix1, iy1, 2, black, true);
    DrawCircle(ix2, iy2, 2, black, true);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
