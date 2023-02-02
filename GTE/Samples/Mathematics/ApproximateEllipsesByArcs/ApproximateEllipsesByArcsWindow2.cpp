// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ApproximateEllipsesByArcsWindow2.h"
#include <Mathematics/ApprEllipseByArcs.h>

ApproximateEllipsesByArcsWindow2::ApproximateEllipsesByArcsWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mA(2.0f),
    mB(1.0f),
    mNumArcs(2),
    mMultiplier(0.375f * mXSize / mA),
    mOffset(Vector2<float>{ 0.5f * mXSize, 0.5f * mYSize })
{
    ApproximateEllipseByArcs(mA, mB, mNumArcs, mPoints, mCenters, mRadii);
    OnDisplay();
}

void ApproximateEllipsesByArcsWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    // Draw the ellipse itself.
    int32_t xCenter = mXSize / 2;
    int32_t yCenter = mYSize / 2;
    int32_t xExtent = static_cast<int32_t>(mMultiplier * mA);
    int32_t yExtent = static_cast<int32_t>(mMultiplier * mB);
    DrawEllipse(xCenter, yCenter, xExtent, yExtent, 0xFFFF0000);

    // Draw the circular arcs that approximate the ellipse.  The drawing is
    // inefficient (not the concern of this sample application) in that it
    // overdraws pixels (unlike a Bresenham-style algorithm).
    size_t const numArcs = static_cast<size_t>(mNumArcs);
    size_t const numArcSamples = static_cast<size_t>(mXSize);
    for (size_t i = 0; i < numArcs; ++i)
    {
        // Get the arc endpoints, center, and radius in screen coordinates.
        Vector2<float> p0 = mPoints[i];
        Vector2<float> p1 = mPoints[i + 1];
        Vector2<float> center = mCenters[i];
        float radius = mRadii[i];

        // Compute the angle between p0 and p1.
        Vector2<float> v0 = p0 - center;
        Vector2<float> v1 = p1 - center;
        float cosAngle = Dot(v0, v1) / (radius * radius);
        cosAngle = std::max(std::min(cosAngle, (float)GTE_C_PI), 0.0f);
        float angle = acos(cosAngle);
        float relativeAngle = angle / static_cast<float>(numArcSamples);

        // Draw the arc in the first quadrant.
        for (size_t j = 0; j <= numArcSamples; ++j)
        {
            float t = static_cast<float>(j) * relativeAngle;
            float cs = std::cos(t), sn = std::sin(t);
            Matrix2x2<float> rot{ cs, -sn, sn, cs };
            Vector2<float> p = center + rot * v0;
            Vector2<float> pScreen = mMultiplier * p + mOffset;
            int32_t x = static_cast<int32_t>(pScreen[0]);
            int32_t y = static_cast<int32_t>(pScreen[1]);
            int32_t rx = mXSize - 1 - x;
            int32_t ry = mYSize - 1 - y;
            SetPixel(x, y, 0xFF0000FF);
            SetPixel(x, ry, 0xFF0000FF);
            SetPixel(rx, y, 0xFF0000FF);
            SetPixel(rx, ry, 0xFF0000FF);
        }
    }

    // Draw the arc endpoints.
    for (auto const& p : mPoints)
    {
        Vector2<float> pScreen = mMultiplier * p + mOffset;
        int32_t x = static_cast<int32_t>(pScreen[0]);
        int32_t y = static_cast<int32_t>(pScreen[1]);
        int32_t rx = mXSize - 1 - x;
        int32_t ry = mYSize - 1 - y;
        DrawThickPixel(x, y, 1, 0xFF000000);
        DrawThickPixel(x, ry, 1, 0xFF000000);
        DrawThickPixel(rx, y, 1, 0xFF000000);
        DrawThickPixel(rx, ry, 1, 0xFF000000);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void ApproximateEllipsesByArcsWindow2::DrawScreenOverlay()
{
    std::string message = "number of arcs = " + std::to_string(mNumArcs);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, message);
}

bool ApproximateEllipsesByArcsWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '+':
    case '=':
        if (mNumArcs < 32)
        {
            ++mNumArcs;
            ApproximateEllipseByArcs(mA, mB, mNumArcs, mPoints, mCenters, mRadii);
            OnDisplay();
        }
        return true;

    case '-':
    case '_':
        if (mNumArcs > 2)
        {
            --mNumArcs;
            ApproximateEllipseByArcs(mA, mB, mNumArcs, mPoints, mCenters, mRadii);
            OnDisplay();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
