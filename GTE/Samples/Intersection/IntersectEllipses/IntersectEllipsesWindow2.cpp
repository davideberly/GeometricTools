// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.7.2023.06.16

#include "IntersectEllipsesWindow2.h"
#include <Mathematics/IntrEllipse2Ellipse2.h>

IntersectEllipsesWindow2::IntersectEllipsesWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mEllipse{},
    mCenter{},
    mMatrix{},
    mQuery{},
    mResult{},
    mCosAngle(numAngles),
    mSinAngle(numAngles),
    mOrigin{ static_cast<double>(mXSize / 2), static_cast<double>(mYSize / 2) },
    mTrnDelta{ 0.1, 1.0, 2.0 },
    mRotDelta{ 0.001, 0.01, 0.1 },
    mSpeed(1),
    mActive(0)
{
    for (size_t i = 0; i < numAngles; ++i)
    {
        double angle = GTE_C_TWO_PI * static_cast<double>(i) / static_cast<double>(numAngles);
        mCosAngle[i] = std::cos(angle);
        mSinAngle[i] = std::sin(angle);
    }

    mEllipse[0].center = { 0.0, 0.0 };
    mEllipse[0].axis[0] = { 1.0, 0.0 };
    mEllipse[0].axis[1] = { 0.0, 1.0 };
    mEllipse[0].extent = { 256.0, 128.0 };
    mQuery.GetStandardForm(mEllipse[0], mCenter[0], mMatrix[0]);
    mEllipse[1].center = { 0.0, 0.0 };
    mEllipse[1].axis[0] = { 1.0, 0.0 };
    mEllipse[1].axis[1] = { 0.0, 1.0 };
    mEllipse[1].extent = { 128.0, 256.0 };
    mQuery.GetStandardForm(mEllipse[1], mCenter[1], mMatrix[1]);
    mResult = mQuery(mEllipse[0], mEllipse[1]);

    mDoFlip = true;
    OnDisplay();
}

void IntersectEllipsesWindow2::OnDisplay()
{
    uint32_t constexpr white = 0xFFFFFFFF;
    uint32_t constexpr red = 0xFF0000FF;
    uint32_t constexpr blue = 0xFFFF0000;
    uint32_t constexpr black = 0xFF000000;

    ClearScreen(white);

    int32_t x0{}, y0{}, x1{}, y1{};

    Get(mEllipse[0], 0, x0, y0);
    for (size_t i = 1; i < numAngles; ++i)
    {
        Get(mEllipse[0], i, x1, y1);
        DrawLine(x0, y0, x1, y1, red);
        x0 = x1;
        y0 = y1;
    }
    Get(mEllipse[0], 0, x1, y1);
    DrawLine(x0, y0, x1, y1, red);

    Get(mEllipse[1], 0, x0, y0);
    for (size_t i = 1; i < numAngles; ++i)
    {
        Get(mEllipse[1], i, x1, y1);
        DrawLine(x0, y0, x1, y1, blue);
        x0 = x1;
        y0 = y1;
    }
    Get(mEllipse[1], 0, x1, y1);
    DrawLine(x0, y0, x1, y1, blue);

    for (size_t i = 0; i < mResult.numPoints; ++i)
    {
        Vector2<double> P = mResult.points[i] + mOrigin;
        x0 = static_cast<int32_t>(P[0]);
        y0 = static_cast<int32_t>(P[1]);
        DrawThickPixel(x0, y0, 2, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void IntersectEllipsesWindow2::DrawScreenOverlay()
{
    std::array<float, 4> black = { 0.0, 0.0, 0.0, 1.0 };

    std::string message = "active ellipse = " + std::to_string(mActive);
    mEngine->Draw(8, 24, black, message);

    if (mSpeed == 0)
    {
        message = "speed = slow";
    }
    else if (mSpeed == 1)
    {
        message = "speed = medium";
    }
    else
    {
        message = "speed = fast";
    }
    mEngine->Draw(8, 48, black, message);
}

bool IntersectEllipsesWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'a':
    case 'A':
        mActive = 1 - mActive;
        OnDisplay();
        return true;

    case 's':
        if (--mSpeed < 0)
        {
            mSpeed = 2;
        }
        OnDisplay();
        return true;
    case 'S':
        if (++mSpeed > 2)
        {
            mSpeed = 0;
        }
        OnDisplay();
        return true;

    case 'x':
        Translate(0, -mTrnDelta[mSpeed]);
        return true;
    case 'X':
        Translate(0, +mTrnDelta[mSpeed]);
        return true;
    case 'y':
        Translate(1, -mTrnDelta[mSpeed]);
        return true;
    case 'Y':
        Translate(1, +mTrnDelta[mSpeed]);
        return true;
    case 'r':
        Rotate(-mRotDelta[mSpeed]);
        return true;
    case 'R':
        Rotate(+mRotDelta[mSpeed]);
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

void IntersectEllipsesWindow2::Translate(int32_t i, double trnDelta)
{
    mEllipse[mActive].center[i] += trnDelta;
    DoQuery();
    OnDisplay();
}

void IntersectEllipsesWindow2::Rotate(double rotDelta)
{
    Matrix2x2<double> rot{};
    MakeRotation(rotDelta, rot);
    mEllipse[mActive].axis[0] = rot * mEllipse[mActive].axis[0];
    mEllipse[mActive].axis[1] = rot * mEllipse[mActive].axis[1];
    DoQuery();
    OnDisplay();
}

void IntersectEllipsesWindow2::DoQuery()
{
    mQuery.GetStandardForm(mEllipse[0], mCenter[0], mMatrix[0]);
    mQuery.GetStandardForm(mEllipse[1], mCenter[1], mMatrix[1]);
    mResult = mQuery(mEllipse[0], mEllipse[1]); // , false);

    std::array<double, 4> test0{}, test1{};
    for (size_t i = 0; i < mResult.numPoints; ++i)
    {
        Vector2<double> delta0 = mResult.points[i] - mCenter[0];
        Vector2<double> delta1 = mResult.points[i] - mCenter[1];
        test0[i] = Dot(delta0, mMatrix[0] * delta0) - 1.0;
        test1[i] = Dot(delta1, mMatrix[1] * delta1) - 1.0;
    }
}
