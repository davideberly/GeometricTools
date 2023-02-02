// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DistanceLine2Box2Window2.h"

#include <Mathematics/DistRay2AlignedBox2.h>
#include <Mathematics/DistRay2OrientedBox2.h>
#include <Mathematics/DistSegment2AlignedBox2.h>
#include <Mathematics/DistSegment2OrientedBox2.h>
namespace gte
{
    template class DCPQuery<double, Ray2<double>, AlignedBox2<double>>;
    template class DCPQuery<double, Ray2<double>, OrientedBox2<double>>;
    template class DCPQuery<double, Segment2<double>, AlignedBox2<double>>;
    template class DCPQuery<double, Segment2<double>, OrientedBox2<double>>;
}

DistanceLine2Box2Window2::DistanceLine2Box2Window2(Parameters& parameters)
    :
    Window2(parameters),
    mLinear{},
    mBox{},
    mQuery{},
    mResult{},
    mAngle(0.0)
#if defined(USE_QUERY_SEGMENT)
    ,
    mSegmentLength(64.0)
#endif
{
#if defined(USE_QUERY_SEGMENT)
    mLinear.p[0] = { 0.5 * mXSize, 0.5 * mYSize };
    mLinear.p[1] = mLinear.p[0] + Vector2<double>{ mSegmentLength, 0.0 };
#else
    mLinear.origin = { 0.5 * mXSize, 0.5 * mYSize };
    mLinear.direction = { std::cos(mAngle), std::sin(mAngle) };
#endif

#if defined(USE_QUERY_ALIGNED_BOX)
    mBox.min = { 200.0, 200.0 };
    mBox.max = mBox.min + Vector2<double>{ 128.0, 64.0 };
#else
    mBox.center = { 264.0, 232.0 };
    double const boxAngle = GTE_C_PI / 6.0;
    mBox.axis[0] = { std::cos(boxAngle), std::sin(boxAngle) };
    mBox.axis[1] = { -std::sin(boxAngle), std::cos(boxAngle) };
    mBox.extent = { 64.0, 32.0 };
#endif

    mResult = mQuery(mLinear, mBox);

    mDoFlip = true;
    OnDisplay();
}

void DistanceLine2Box2Window2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const black = 0xFF000000;
    uint32_t const red = 0xFF0000FF;
    uint32_t const green = 0xFF00FF00;
    uint32_t const blue = 0xFFFF0000;

    ClearScreen(white);

    int32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;

#if defined(USE_QUERY_ALIGNED_BOX)
    x0 = (int32_t)mBox.min[0];
    y0 = (int32_t)mBox.min[1];
    x1 = (int32_t)mBox.max[0];
    y1 = (int32_t)mBox.max[1];
    DrawRectangle(x0, y0, x1, y1, black, false);
#else
    std::array<Vector2<double>, 4> vertices{};
    mBox.GetVertices(vertices);
    x0 = (int32_t)vertices[0][0];
    y0 = (int32_t)vertices[0][1];
    x1 = (int32_t)vertices[1][0];
    y1 = (int32_t)vertices[1][1];
    DrawLine(x0, y0, x1, y1, black);
    x1 = (int32_t)vertices[2][0];
    y1 = (int32_t)vertices[2][1];
    DrawLine(x0, y0, x1, y1, black);
    x0 = (int32_t)vertices[3][0];
    y0 = (int32_t)vertices[3][1];
    DrawLine(x0, y0, x1, y1, black);
    x1 = (int32_t)vertices[1][0];
    y1 = (int32_t)vertices[1][1];
    DrawLine(x0, y0, x1, y1, black);
#endif

#if defined(USE_QUERY_LINE)
    Vector2<double> linearMin = mLinear.origin - 512.0 * mLinear.direction;
    Vector2<double> linearMax = mLinear.origin + 512.0 * mLinear.direction;
#endif

#if defined(USE_QUERY_RAY)
    Vector2<double> linearMin = mLinear.origin;
    Vector2<double> linearMax = mLinear.origin + 512.0 * mLinear.direction;
#endif

#if defined(USE_QUERY_SEGMENT)
    Vector2<double> linearMin = mLinear.p[0];
    Vector2<double> linearMax = mLinear.p[1];
#endif

    x0 = (int32_t)linearMin[0];
    y0 = (int32_t)linearMin[1];
    x1 = (int32_t)linearMax[0];
    y1 = (int32_t)linearMax[1];
    DrawLine(x0, y0, x1, y1, blue);

    x0 = (int32_t)mResult.closest[0][0];
    y0 = (int32_t)mResult.closest[0][1];
    x1 = (int32_t)mResult.closest[1][0];
    y1 = (int32_t)mResult.closest[1][1];
    DrawThickPixel(x0, y0, 1, red);
    DrawThickPixel(x1, y1, 1, green);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void DistanceLine2Box2Window2::DrawScreenOverlay()
{
    std::string message = "distance = " + std::to_string(mResult.distance);
    mEngine->Draw(8, 24, { 0.0f, 0.0f, 0.0f, 1.0f }, message);
}

bool DistanceLine2Box2Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    double const trnDelta = 1.0;
    double const rotDelta = GTE_C_DEG_TO_RAD;

    switch (key)
    {
    case 'q':
        mResult = mQuery(mLinear, mBox);
        return true;
#if defined(USE_QUERY_SEGMENT)
    case 'x':
        mLinear.p[0][0] -= trnDelta;
        mLinear.p[1][0] -= trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'X':
        mLinear.p[0][0] += trnDelta;
        mLinear.p[1][0] += trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'y':
        mLinear.p[0][1] -= trnDelta;
        mLinear.p[1][1] -= trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'Y':
        mLinear.p[0][1] += trnDelta;
        mLinear.p[1][1] += trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'r':
        mAngle -= rotDelta;
        mLinear.p[1] = mLinear.p[0] +
            mSegmentLength * Vector2<double>{ std::cos(mAngle), std::sin(mAngle) };
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'R':
        mAngle += rotDelta;
        mLinear.p[1] = mLinear.p[0] +
            mSegmentLength * Vector2<double>{ std::cos(mAngle), std::sin(mAngle) };
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
#else
    case 'x':
        mLinear.origin[0] -= trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'X':
        mLinear.origin[0] += trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'y':
        mLinear.origin[1] -= trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'Y':
        mLinear.origin[1] += trnDelta;
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'r':
        mAngle -= rotDelta;
        mLinear.direction = { std::cos(mAngle), std::sin(mAngle) };
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
    case 'R':
        mAngle += rotDelta;
        mLinear.direction = { std::cos(mAngle), std::sin(mAngle) };
        mResult = mQuery(mLinear, mBox);
        OnDisplay();
        return true;
#endif
    }
    return Window2::OnCharPress(key, x, y);
}
