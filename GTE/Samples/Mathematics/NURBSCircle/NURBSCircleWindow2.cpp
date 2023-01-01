// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "NURBSCircleWindow2.h"

NURBSCircleWindow2::NURBSCircleWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    mDoFlip = true;
    OnDisplay();
}

void NURBSCircleWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int32_t const dx = mXSize / 4;
    int32_t const dy = mYSize / 4;
    int32_t const radius = mXSize / 8;

    float const halfPi = static_cast<float>(GTE_C_HALF_PI);
    float const pi = static_cast<float>(GTE_C_PI);
    float const twoPi = static_cast<float>(GTE_C_TWO_PI);
    DrawCurve(&mQuarterCircleDegree2, halfPi, dx, dy, radius);
    DrawCurve(&mQuarterCircleDegree4, halfPi, 3 * dx, dy, radius);
    DrawCurve(&mHalfCircleDegree3, pi, dx, 3 * dy, radius);
    DrawCurve(&mFullCircleDegree3, twoPi, 3 * dx, 3 * dy, radius);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void NURBSCircleWindow2::DrawCurve(NURBSCurve<2, float> const* curve, float maxAngle,
    int32_t iXCenter, int32_t iYCenter, int32_t iRadius)
{
    Vector2<float> const center{ static_cast<float>(iXCenter), static_cast<float>(iYCenter) };
    float const radius = static_cast<float>(iRadius);
    int32_t x0, y0, x1, y1;

    // Draw the true circle in green with thickness to allow contrast between
    // the true circle and the NURBS circle.
    int32_t const numSamples = 1024;
    x0 = static_cast<int32_t>(std::lrint(center[0] + radius));
    y0 = static_cast<int32_t>(std::lrint(center[1]));
    float const divisor = static_cast<float>(numSamples - 1);
    for (int32_t i = 1; i < numSamples; ++i)
    {
        float angle = maxAngle * static_cast<float>(i) / divisor;
        float cs = std::cos(angle);
        float sn = std::sin(angle);
        x1 = static_cast<int32_t>(std::lrint(center[0] + radius * cs));
        y1 = static_cast<int32_t>(std::lrint(center[1] + radius * sn));
        DrawThickLine(x0, y0, x1, y1, 1, 0xFF00FF00);
        x0 = x1;
        y0 = y1;
    }

    // Draw the NURBS circle in blue.
    Vector2<float> values[4];
    curve->Evaluate(0.0f, 0, values);
    x0 = static_cast<int32_t>(std::lrint(center[0] + radius * values[0][0]));
    y0 = static_cast<int32_t>(std::lrint(center[1] + radius * values[0][1]));
    for (int32_t i = 1; i < numSamples; ++i)
    {
        float t = static_cast<float>(i) / divisor;
        curve->Evaluate(t, 0, values);
        x1 = static_cast<int32_t>(std::lrint(center[0] + radius * values[0][0]));
        y1 = static_cast<int32_t>(std::lrint(center[1] + radius * values[0][1]));
        DrawLine(x0, y0, x1, y1, 0xFFFF0000);
        x0 = x1;
        y0 = y1;
    }
}
