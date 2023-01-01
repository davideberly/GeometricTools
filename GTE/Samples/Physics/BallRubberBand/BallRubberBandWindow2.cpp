// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BallRubberBandWindow2.h"

BallRubberBandWindow2::BallRubberBandWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    // Set up the physics module.
    mModule.springConstant = 16.0f;
    mModule.mass = 1.0f;
    mModule.Initialize(0.0f, 0.01f, { 96.0f, 96.0f }, { 64.0f, 0.0f });

    mPosition.resize(128);
    for (auto& position : mPosition)
    {
        position = mModule.GetPosition();
        mModule.Update();
    }

    OnDisplay();
}

void BallRubberBandWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int32_t const halfSize = mXSize / 2, sizeM1 = mXSize - 1;
    float const fHalfSize = static_cast<float>(halfSize);

    // Draw the coordinate axes.
    DrawLine(0, halfSize, sizeM1, halfSize, 0xFFC0C0C0);
    DrawLine(halfSize, 0, halfSize, sizeM1, 0xFFC0C0C0);

    // Draw the ball's path.  The orbit starts in green, finishes in blue,
    // and is a blend of the two colors between.
    size_t const numPositions = mPosition.size();
    float const invNumPositions = 1.0f / static_cast<float>(numPositions);
    for (size_t i = 0; i + 1 < numPositions; ++i)
    {
        float w = static_cast<float>(i) * invNumPositions, omw = 1.0f - w;
        uint32_t blue = static_cast<uint32_t>(255.0f * omw);
        uint32_t green = static_cast<uint32_t>(255.0f * w);
        uint32_t color = (green << 8) | (blue << 16) | 0xFF000000;
        int32_t x0 = static_cast<int32_t>(mPosition[i][0] + fHalfSize);
        int32_t y0 = static_cast<int32_t>(mPosition[i][1] + fHalfSize);
        int32_t x1 = static_cast<int32_t>(mPosition[i + 1][0] + fHalfSize);
        int32_t y1 = static_cast<int32_t>(mPosition[i + 1][1] + fHalfSize);
        DrawLine(x0, y0, x1, y1, color);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}
