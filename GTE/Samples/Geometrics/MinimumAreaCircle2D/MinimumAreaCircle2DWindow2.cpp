// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MinimumAreaCircle2DWindow2.h"

MinimumAreaCircle2DWindow2::MinimumAreaCircle2DWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mNumActive(2),
    mVertices(NUM_POINTS)
{
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.25f*mXSize, 0.75f*mYSize);
    for (auto& v : mVertices)
    {
        v = { rnd(mte), rnd(mte) };
    }

    mMAC2(mNumActive, &mVertices[0], mMinimalCircle);
}

void MinimumAreaCircle2DWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const gray = 0xFF808080;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const red = 0xFF0000FF;

    // Draw the minimum area circle.
    int32_t x = static_cast<int32_t>(std::lrint(mMinimalCircle.center[0]));
    int32_t y = static_cast<int32_t>(std::lrint(mMinimalCircle.center[1]));
    int32_t radius = static_cast<int32_t>(std::lrint(mMinimalCircle.radius));
    DrawCircle(x, y, radius, gray, false);

    // Draw the support.
    int32_t numSupport = mMAC2.GetNumSupport();
    std::array<int32_t, 3> support = mMAC2.GetSupport();
    for (int32_t i0 = numSupport - 1, i1 = 0; i1 <numSupport; i0 = i1++)
    {
        int32_t x0 = static_cast<int32_t>(std::lrint(mVertices[support[i0]][0]));
        int32_t y0 = static_cast<int32_t>(std::lrint(mVertices[support[i0]][1]));
        int32_t x1 = static_cast<int32_t>(std::lrint(mVertices[support[i1]][0]));
        int32_t y1 = static_cast<int32_t>(std::lrint(mVertices[support[i1]][1]));
        DrawLine(x0, y0, x1, y1, red);
    }

    // Draw the input points.
    for (int32_t i = 0; i < mNumActive; ++i)
    {
        x = static_cast<int32_t>(std::lrint(mVertices[i][0]));
        y = static_cast<int32_t>(std::lrint(mVertices[i][1]));
        DrawThickPixel(x, y, 1, blue);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MinimumAreaCircle2DWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'n':
    case 'N':
        if (mNumActive < NUM_POINTS)
        {
            mMAC2(++mNumActive, &mVertices[0], mMinimalCircle);
            OnDisplay();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
