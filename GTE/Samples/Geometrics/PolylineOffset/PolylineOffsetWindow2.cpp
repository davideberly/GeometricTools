// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2022.12.24

#include "PolylineOffsetWindow2.h"

namespace gte
{
    template class PolylineOffset<float>;
}

PolylineOffsetWindow2::PolylineOffsetWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mVertices(16),
    mIsOpen(true),
    mOffseter{},
    mRightPolyline{},
    mLeftPolyline{},
    mOffsetDistance(8.0)
{
    // Choose vertices on the spiral r = theta (polar coordinates) where the
    // angle samples are theta = (2 * pi * k) / mVertices.size() for
    // 0 <= k < mVertices.size().
    // 
    // Scale the spiral to fill a large portion of the window and then
    // translate the origin to the center point of the window. The coordinates
    // are
    //   x = 128 * r * cos(theta) + 384
    //   y = 128 * r * sin(theta) + 384
    double const dsize = static_cast<double>(mVertices.size());
    double constexpr translate = 384.0;
    double constexpr scale = 64;
    for (size_t k = 0; k < mVertices.size(); ++k)
    {
        double theta = GTE_C_TWO_PI * static_cast<double>(k) / dsize;
        mVertices[k][0] = scale * theta * std::cos(theta) + translate;
        mVertices[k][1] = scale * theta * std::sin(theta) + translate;
    }

    mOffseter = std::make_shared<PolylineOffset<double>>(mVertices, mIsOpen);
    mOffseter->Execute(mOffsetDistance, true, mRightPolyline, true, mLeftPolyline);

    mDoFlip = true;
    OnDisplay();
}

void PolylineOffsetWindow2::OnDisplay()
{
    uint32_t constexpr white = 0xFFFFFFFF;
    uint32_t constexpr gray = 0xFF808080;
    uint32_t constexpr blue = 0xFFFF0000;
    uint32_t constexpr red = 0xFF0000FF;

    ClearScreen(white);

    int32_t x0{}, y0{}, x1{}, y1{};

    if (mIsOpen)
    {
        // Draw the open polyline.
        for (size_t k0 = 0, k1 = 1; k1 < mVertices.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mVertices[k0][0]);
            y0 = static_cast<int32_t>(mVertices[k0][1]);
            x1 = static_cast<int32_t>(mVertices[k1][0]);
            y1 = static_cast<int32_t>(mVertices[k1][1]);
            DrawLine(x0, y0, x1, y1, gray);
        }

        // Draw the right polyline.
        for (size_t k0 = 0, k1 = 1; k1 < mRightPolyline.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mRightPolyline[k0][0]);
            y0 = static_cast<int32_t>(mRightPolyline[k0][1]);
            x1 = static_cast<int32_t>(mRightPolyline[k1][0]);
            y1 = static_cast<int32_t>(mRightPolyline[k1][1]);
            DrawLine(x0, y0, x1, y1, blue);
        }

        // Draw the left polyline.
        for (size_t k0 = 0, k1 = 1; k1 < mLeftPolyline.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mLeftPolyline[k0][0]);
            y0 = static_cast<int32_t>(mLeftPolyline[k0][1]);
            x1 = static_cast<int32_t>(mLeftPolyline[k1][0]);
            y1 = static_cast<int32_t>(mLeftPolyline[k1][1]);
            DrawLine(x0, y0, x1, y1, red);
        }
    }
    else
    {
        // Draw the closed polyline.
        for (size_t k0 = mVertices.size() - 1, k1 = 0; k1 < mVertices.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mVertices[k0][0]);
            y0 = static_cast<int32_t>(mVertices[k0][1]);
            x1 = static_cast<int32_t>(mVertices[k1][0]);
            y1 = static_cast<int32_t>(mVertices[k1][1]);
            DrawLine(x0, y0, x1, y1, gray);
        }

        // Draw the right polyline.
        for (size_t k0 = mVertices.size() - 1, k1 = 0; k1 < mRightPolyline.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mRightPolyline[k0][0]);
            y0 = static_cast<int32_t>(mRightPolyline[k0][1]);
            x1 = static_cast<int32_t>(mRightPolyline[k1][0]);
            y1 = static_cast<int32_t>(mRightPolyline[k1][1]);
            DrawLine(x0, y0, x1, y1, blue);
        }

        // Draw the left polyline.
        for (size_t k0 = mVertices.size() - 1, k1 = 0; k1 < mLeftPolyline.size(); k0 = k1++)
        {
            x0 = static_cast<int32_t>(mLeftPolyline[k0][0]);
            y0 = static_cast<int32_t>(mLeftPolyline[k0][1]);
            x1 = static_cast<int32_t>(mLeftPolyline[k1][0]);
            y1 = static_cast<int32_t>(mLeftPolyline[k1][1]);
            DrawLine(x0, y0, x1, y1, red);
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void PolylineOffsetWindow2::DrawScreenOverlay()
{
    std::array<float, 4> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    std::string message{};
    if (mIsOpen)
    {
        message = "polyline is open";
    }
    else
    {
        message = "polyline is closed";
    }
    mEngine->Draw(8, 24, black, message);
    message = "offset distance " + std::to_string(mOffsetDistance);
    mEngine->Draw(8, 48, black, message);
}

bool PolylineOffsetWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case ' ':
        mIsOpen = !mIsOpen;
        mOffseter = std::make_shared<PolylineOffset<double>>(mVertices, mIsOpen);
        mOffseter->Execute(mOffsetDistance, true, mRightPolyline, true, mLeftPolyline);
        OnDisplay();
        return true;

    case '+':
    case '=':
        mOffsetDistance += 1.0;
        mOffseter = std::make_shared<PolylineOffset<double>>(mVertices, mIsOpen);
        mOffseter->Execute(mOffsetDistance, true, mRightPolyline, true, mLeftPolyline);
        OnDisplay();
        return true;

    case '-':
    case '_':
        if (mOffsetDistance > 1.0)
        {
            mOffsetDistance -= 1.0;
            mOffseter = std::make_shared<PolylineOffset<double>>(mVertices, mIsOpen);
            mOffseter->Execute(mOffsetDistance, true, mRightPolyline, true, mLeftPolyline);
            OnDisplay();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
