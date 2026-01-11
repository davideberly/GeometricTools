// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "NURBSCircularArcWindow2.h"

NURBSCircularArcWindow2::NURBSCircularArcWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSampler{},
    mArc{},
    mPoints{},
    mSelection(1)
{
    mArc.center = { static_cast<float>(mXSize / 2 + 50), static_cast<float>(mYSize / 2 + 25) };
    mArc.radius = 175.0f;

    mDoFlip = true;
    OnCharPress('1', 0, 0);
}

void NURBSCircularArcWindow2::OnIdle()
{
    OnDisplay();
}

void NURBSCircularArcWindow2::OnDisplay()
{
    std::uint32_t constexpr white = 0xFFFFFFFF;
    std::uint32_t constexpr blue = 0xFFFF0000;
    std::uint32_t constexpr red = 0xFF0000FF;

    ClearScreen(white);

    DrawCircle(mXSize / 2 + 50, mYSize / 2 + 25, 175, blue, false);
    for (auto const& point : mPoints)
    {
        std::int32_t x = static_cast<std::int32_t>(point[0]);
        std::int32_t y = static_cast<std::int32_t>(point[1]);
        DrawThickPixel(x, y, 1, red);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool NURBSCircularArcWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float angle0{}, angle1{};

    switch (key)
    {
    case '1':
        angle0 = static_cast<float>(GTE_C_PI / 6.0);
        angle1 = static_cast<float>(3.0 * GTE_C_PI / 8.0);
        mArc.end[0] = { std::cos(angle0), std::sin(angle0) };
        mArc.end[1] = { std::cos(angle1), std::sin(angle1) };
        mArc.end[0] = mArc.center + mArc.radius * mArc.end[0];
        mArc.end[1] = mArc.center + mArc.radius * mArc.end[1];
        mSampler(mArc, mPoints);
        OnDisplay();
        return true;

    case '2':
        angle0 = static_cast<float>(GTE_C_PI / 6.0);
        angle1 = static_cast<float>(3.0 * GTE_C_PI / 4.0);
        mArc.end[0] = { std::cos(angle0), std::sin(angle0) };
        mArc.end[1] = { std::cos(angle1), std::sin(angle1) };
        mArc.end[0] = mArc.center + mArc.radius * mArc.end[0];
        mArc.end[1] = mArc.center + mArc.radius * mArc.end[1];
        mSampler(mArc, mPoints);
        OnDisplay();
        return true;

    case '3':
        angle0 = static_cast<float>(GTE_C_PI / 6.0);
        angle1 = static_cast<float>(5.0 * GTE_C_PI / 4.0);
        mArc.end[0] = { std::cos(angle0), std::sin(angle0) };
        mArc.end[1] = { std::cos(angle1), std::sin(angle1) };
        mArc.end[0] = mArc.center + mArc.radius * mArc.end[0];
        mArc.end[1] = mArc.center + mArc.radius * mArc.end[1];
        mSampler(mArc, mPoints);
        OnDisplay();
        return true;

    case '4':
        angle0 = static_cast<float>(GTE_C_PI / 6.0);
        angle1 = static_cast<float>(15.0 * GTE_C_PI / 8.0);
        mArc.end[0] = { std::cos(angle0), std::sin(angle0) };
        mArc.end[1] = { std::cos(angle1), std::sin(angle1) };
        mArc.end[0] = mArc.center + mArc.radius * mArc.end[0];
        mArc.end[1] = mArc.center + mArc.radius * mArc.end[1];
        mSampler(mArc, mPoints);
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}


