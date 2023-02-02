// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "KeplerPolarFormWindow2.h"

KeplerPolarFormWindow2::KeplerPolarFormWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(mXSize)
{
    // Set up the physics module.
    mModule.gravity = 10.0f;
    mModule.mass = 1.0f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float theta = static_cast<float>(GTE_C_QUARTER_PI);
    float thetaDot = 0.1f;
    float radius = 10.0f;
    float radiusDot = 0.1f;
    mModule.Initialize(time, deltaTime, theta, thetaDot, radius, radiusDot);

    int32_t const imax = static_cast<int32_t>(mModule.GetPeriod() / deltaTime);
    mPositions.resize(imax);
    for (int32_t i = 0; i < imax; ++i)
    {
        float x = 0.5f * mSize + 10.0f * radius * std::cos(theta);
        float y = 0.5f * mSize + 10.0f * radius * std::sin(theta);
        mPositions[i] = { x, y };
        mModule.Update();
        theta = mModule.GetTheta();
    }

    mDoFlip = true;
    OnDisplay();
}

void KeplerPolarFormWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int32_t const halfSize = static_cast<int32_t>(0.5f * mSize);
    int32_t const sizeM1 = static_cast<int32_t>(mSize - 1.0f);

    // Draw the coordinate axes.
    uint32_t const gray = 0xFFC0C0C0;
    DrawLine(0, halfSize, sizeM1, halfSize, gray);
    DrawLine(halfSize, 0, halfSize, sizeM1, gray);

    // Draw a ray from the Sun's location to the initial point.
    int32_t x = static_cast<int32_t>(std::lrint(mPositions[1][0]));
    int32_t y = static_cast<int32_t>(std::lrint(mPositions[1][0]));
    DrawLine(halfSize, halfSize, x, y, gray);

    // Draw the Sun's location.  The Sun is at the origin which happens to
    // be a focal point of the ellipse.
    uint32_t const red = 0xFF0000FF;
    DrawThickPixel(halfSize, halfSize, 1, red);

    // Draw Earth's orbit.  The orbit starts in green, finishes in blue, and
    // is a blend of the two colors between.
    int32_t const numPositions = static_cast<int32_t>(mPositions.size());
    float const invNumPositions = 1.0f / static_cast<float>(numPositions);
    for (int32_t i = 1; i < numPositions; ++i)
    {
        float w = static_cast<float>(i) * invNumPositions;
        float oneMinusW = 1.0f - w;
        uint32_t blue = static_cast<uint32_t>(255.0f * oneMinusW);
        uint32_t green = static_cast<uint32_t>(255.0f * w);
        x = static_cast<int32_t>(std::lrint(mPositions[i][0]));
        y = static_cast<int32_t>(std::lrint(mPositions[i][1]));
        uint32_t color = 0xFF000000 | (blue << 16) | (green << 8);
        SetPixel(x, y, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
