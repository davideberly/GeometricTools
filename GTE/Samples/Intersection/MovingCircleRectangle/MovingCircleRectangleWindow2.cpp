// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MovingCircleRectangleWindow2.h"

MovingCircleRectangleWindow2::MovingCircleRectangleWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mLeftMouseDown(false),
    mRightMouseDown(false),
    mHasIntersection(false)
{
    Vector2<double> bmin = { 0.25 * mXSize, 0.375 * mYSize };
    Vector2<double> bmax = { 0.75 * mXSize, 0.625 * mYSize };
    mBox.center = 0.5 * (bmax + bmin);
    mBox.axis[0] = { 1.0, 0.0 };
    mBox.axis[1] = { 0.0, 1.0 };
    mBox.extent = 0.5 * (bmax - bmin);
    mBoxVelocity = { 0.0, 0.0 };

    mCircle.center = { 0.9 * mXSize, 0.9 * mYSize };
    mCircle.radius = 16.0;
    mCircleVelocity = { 1.0, 0.0 };

    mContactTime = 0.0;
    mContactPoint = { 0.0, 0.0 };

    mDoFlip = true;
    DoQuery();
}

void MovingCircleRectangleWindow2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFFF0F0F0;
    uint32_t const red = 0xFF0000FF;
    uint32_t const green = 0xFF00FF00;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const orange = 0xFF0080FF;

    int32_t bx0 = static_cast<int32_t>(mBox.center[0]);
    int32_t bx1 = static_cast<int32_t>(mBox.center[1]);

    // K = { C-e0*U0-e1*U1, C+e0*U0-e1*U1, C-e0*U0+e1*U1, C+e0*U0+e1*U1 }
    std::array<Vector2<double>, 4> K;
    mBox.GetVertices(K);

    int32_t cx0 = static_cast<int32_t>(mCircle.center[0]);
    int32_t cy0 = static_cast<int32_t>(mCircle.center[1]);
    int32_t r = static_cast<int32_t>(mCircle.radius);

    ClearScreen(white);

    // Draw the rounded rectangle.
    for (int32_t i = 0; i < 4; ++i)
    {
        DrawCircle(static_cast<int32_t>(K[i][0]), static_cast<int32_t>(K[i][1]), r, gray, true);
    }

    Vector2<double> T0 = K[0] - mCircle.radius * mBox.axis[1];
    Vector2<double> T1 = K[1] - mCircle.radius * mBox.axis[1];
    DrawLine(static_cast<int32_t>(T0[0]), static_cast<int32_t>(T0[1]),
        static_cast<int32_t>(T1[0]), static_cast<int32_t>(T1[1]), gray);
    T0 = K[2] + mCircle.radius * mBox.axis[1];
    T1 = K[3] + mCircle.radius * mBox.axis[1];
    DrawLine(static_cast<int32_t>(T0[0]), static_cast<int32_t>(T0[1]),
        static_cast<int32_t>(T1[0]), static_cast<int32_t>(T1[1]), gray);
    T0 = K[0] - mCircle.radius * mBox.axis[0];
    T1 = K[2] - mCircle.radius * mBox.axis[0];
    DrawLine(static_cast<int32_t>(T0[0]), static_cast<int32_t>(T0[1]),
        static_cast<int32_t>(T1[0]), static_cast<int32_t>(T1[1]), gray);
    T0 = K[1] + mCircle.radius * mBox.axis[0];
    T1 = K[3] +mCircle.radius * mBox.axis[0];
    DrawLine(static_cast<int32_t>(T0[0]), static_cast<int32_t>(T0[1]),
        static_cast<int32_t>(T1[0]), static_cast<int32_t>(T1[1]), gray);
    DrawFloodFill4(bx0, bx1, gray, white);

    // Draw the rectangle.
    DrawLine(static_cast<int32_t>(K[0][0]), static_cast<int32_t>(K[0][1]),
        static_cast<int32_t>(K[1][0]), static_cast<int32_t>(K[1][1]), blue);
    DrawLine(static_cast<int32_t>(K[1][0]), static_cast<int32_t>(K[1][1]),
        static_cast<int32_t>(K[3][0]), static_cast<int32_t>(K[3][1]), blue);
    DrawLine(static_cast<int32_t>(K[3][0]), static_cast<int32_t>(K[3][1]),
        static_cast<int32_t>(K[2][0]), static_cast<int32_t>(K[2][1]), blue);
    DrawLine(static_cast<int32_t>(K[2][0]), static_cast<int32_t>(K[2][1]),
        static_cast<int32_t>(K[0][0]), static_cast<int32_t>(K[0][1]), blue);

    // Draw the initial circle.
    DrawCircle(cx0, cy0, r, red, false);

    // Draw velocity ray with origin at the circle center.
    int32_t cx1 = cx0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[0]);
    int32_t cy1 = cy0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, green);

    // Draw parallel velocity rays that are tangent to the circle.
    Vector2<double> vPerp = UnitPerp(mCircleVelocity);
    Vector2<double> origin = mCircle.center + mCircle.radius * vPerp;
    cx0 = static_cast<int32_t>(origin[0]);
    cy0 = static_cast<int32_t>(origin[1]);
    cx1 = cx0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[0]);
    cy1 = cy0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, orange);

    origin = mCircle.center - mCircle.radius * vPerp;
    cx0 = static_cast<int32_t>(origin[0]);
    cy0 = static_cast<int32_t>(origin[1]);
    cx1 = cx0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[0]);
    cy1 = cy0 + static_cast<int32_t>((2 * mXSize) * mCircleVelocity[1]);
    DrawLine(cx0, cy0, cx1, cy1, orange);

    if (mHasIntersection)
    {
        // Draw the circle at time of contact.
        cx0 = static_cast<int32_t>(mCircle.center[0] + mContactTime * mCircleVelocity[0]);
        cy0 = static_cast<int32_t>(mCircle.center[1] + mContactTime * mCircleVelocity[1]);
        DrawCircle(cx0, cy0, r, black, false);

        int32_t px = static_cast<int32_t>(mContactPoint[0]);
        int32_t py = static_cast<int32_t>(mContactPoint[1]);
        DrawThickPixel(px, py, 1, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MovingCircleRectangleWindow2::OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers)
{
    if (button == MOUSE_LEFT)
    {
        mLeftMouseDown = (state == MOUSE_DOWN);
        ModifyVelocity(x, mYSize - 1 - y);
        return true;
    }

    if (button == MOUSE_RIGHT)
    {
        mRightMouseDown = (state == MOUSE_DOWN);
        ModifyCircle(x, mYSize - 1 - y);
        return true;
    }

    return Window2::OnMouseClick(button, state, x, y, modifiers);
}

bool MovingCircleRectangleWindow2::OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers)
{
    if (button == MOUSE_LEFT)
    {
        if (mLeftMouseDown)
        {
            ModifyVelocity(x, mYSize - 1 - y);
            return true;
        }
    }

    if (button == MOUSE_RIGHT)
    {
        if (mRightMouseDown)
        {
            ModifyCircle(x, mYSize - 1 - y);
            return true;
        }
    }

    return Window2::OnMouseMotion(button, x, y, modifiers);
}

bool MovingCircleRectangleWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    if (key == '-' || key == '_')
    {
        ModifyRectangle(-1.0);
        return true;
    }

    if (key == '+' || key == '=')
    {
        ModifyRectangle(+1.0);
        return true;
    }

    if (key == ' ')
    {
        auto result = mQuery(mBox, mBoxVelocity, mCircle, mCircleVelocity);
        mHasIntersection = (result.intersectionType != 0);
        if (mHasIntersection)
        {
            mContactTime = result.contactTime;
            mContactPoint = result.contactPoint;
        }
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

void MovingCircleRectangleWindow2::DoQuery()
{
    auto result = mQuery(mBox, mBoxVelocity, mCircle, mCircleVelocity);
    mHasIntersection = (result.intersectionType != 0);
    if (mHasIntersection)
    {
        mContactTime = result.contactTime;
        mContactPoint = result.contactPoint;
    }

    OnDisplay();
}

void MovingCircleRectangleWindow2::ModifyVelocity(int32_t x, int32_t y)
{
    int32_t cx = static_cast<int32_t>(mCircle.center[0]);
    int32_t cy = static_cast<int32_t>(mCircle.center[1]);
    mCircleVelocity[0] = static_cast<double>(x) - static_cast<double>(cx);
    mCircleVelocity[1] = static_cast<double>(y) - static_cast<double>(cy);
    Normalize(mCircleVelocity);
    DoQuery();
}

void MovingCircleRectangleWindow2::ModifyCircle(int32_t x, int32_t y)
{
    mCircle.center[0] = static_cast<double>(x);
    mCircle.center[1] = static_cast<double>(y);
    DoQuery();
}

void MovingCircleRectangleWindow2::ModifyRectangle(double direction)
{
    // Rotate the box by one degree.
    double const angle = direction * GTE_C_DEG_TO_RAD;
    double cs = std::cos(angle), sn = std::sin(angle);
    Vector2<double> temp0 = mBox.axis[0];
    Vector2<double> temp1 = mBox.axis[1];
    mBox.axis[0] = cs * temp0 - sn * temp1;
    mBox.axis[1] = sn * temp0 + cs * temp1;
    DoQuery();
}
