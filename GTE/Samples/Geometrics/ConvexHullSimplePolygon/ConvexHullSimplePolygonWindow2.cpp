// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "ConvexHullSimplePolygonWindow2.h"

ConvexHullSimplePolygonWindow2::ConvexHullSimplePolygonWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mPolygon{},
    mOrder{},
    mHull{},
    mCHSP{}
{
    Polygon0StartConvex();
    mDoFlip = true;
    OnDisplay();
}

void ConvexHullSimplePolygonWindow2::OnDisplay()
{
    std::uint32_t constexpr white = 0xFFFFFFFF;
    std::uint32_t constexpr blue = 0xFFFF0000;
    std::uint32_t constexpr red = 0xFF0000FF;
    ClearScreen(white);

    std::int32_t x0{}, y0{}, x1{}, y1{};
    std::int32_t xOffset = 0;

    x0 = static_cast<std::int32_t>(mPolygon[0][0]) + xOffset;
    y0 = static_cast<std::int32_t>(mPolygon[0][1]);
    for (std::size_t i = 1; i < mPolygon.size(); ++i)
    {
        x1 = static_cast<std::int32_t>(mPolygon[i][0]) + xOffset;
        y1 = static_cast<std::int32_t>(mPolygon[i][1]);
        DrawLine(x0, y0, x1, y1, blue);
        x0 = x1;
        y0 = y1;
    }
    x1 = static_cast<std::int32_t>(mPolygon[0][0]) + xOffset;
    y1 = static_cast<std::int32_t>(mPolygon[0][1]);
    DrawLine(x0, y0, x1, y1, blue);

    xOffset = 400;
    x0 = static_cast<std::int32_t>(mPolygon[0][0]) + xOffset;
    y0 = static_cast<std::int32_t>(mPolygon[0][1]);
    for (std::size_t i = 1; i < mPolygon.size(); ++i)
    {
        x1 = static_cast<std::int32_t>(mPolygon[i][0]) + xOffset;
        y1 = static_cast<std::int32_t>(mPolygon[i][1]);
        DrawLine(x0, y0, x1, y1, blue);
        x0 = x1;
        y0 = y1;
    }
    x1 = static_cast<std::int32_t>(mPolygon[0][0]) + xOffset;
    y1 = static_cast<std::int32_t>(mPolygon[0][1]);
    DrawLine(x0, y0, x1, y1, blue);

    x0 = static_cast<std::int32_t>(mPolygon[mHull[0]][0]) + xOffset;
    y0 = static_cast<std::int32_t>(mPolygon[mHull[0]][1]);
    for (std::size_t i = 1; i < mHull.size(); ++i)
    {
        x1 = static_cast<std::int32_t>(mPolygon[mHull[i]][0]) + xOffset;
        y1 = static_cast<std::int32_t>(mPolygon[mHull[i]][1]);
        DrawLine(x0, y0, x1, y1, red);
        x0 = x1;
        y0 = y1;
    }
    x0 = static_cast<std::int32_t>(mPolygon[mHull[0]][0]) + xOffset;
    y0 = static_cast<std::int32_t>(mPolygon[mHull[0]][1]);
    DrawLine(x0, y0, x1, y1, red);

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool ConvexHullSimplePolygonWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        Polygon0StartConvex();
        OnDisplay();
        return true;
    case '1':
        Polygon0StartReflex();
        OnDisplay();
        return true;
    case '2':
        Polygon1StartConvex();
        OnDisplay();
        return true;
    case '3':
        Polygon1StartReflex();
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

void ConvexHullSimplePolygonWindow2::Polygon0StartConvex()
{
    // Start with convex vertex.
    mPolygon.resize(8);
    mPolygon[0] = { 128.0f, 256.0f };
    mPolygon[1] = { 200.0f, 100.0f };
    mPolygon[2] = { 256.0f, 150.0f };
    mPolygon[3] = { 400.0f, 100.0f };
    mPolygon[4] = { 400.0f, 400.0f };
    mPolygon[5] = { 300.0f, 300.0f };
    mPolygon[6] = { 230.0f, 300.0f };
    mPolygon[7] = { 220.0f, 450.0f };

    LogAssert(
        mOrder(mPolygon),
        "The polygon must be counterclockwise.");

    mCHSP(mPolygon, mHull);
}

void ConvexHullSimplePolygonWindow2::Polygon0StartReflex()
{
    // Start with reflex vertex.
    mPolygon.resize(8);
    mPolygon[0] = { 256.0f, 150.0f };
    mPolygon[1] = { 400.0f, 100.0f };
    mPolygon[2] = { 400.0f, 400.0f };
    mPolygon[3] = { 300.0f, 300.0f };
    mPolygon[4] = { 230.0f, 300.0f };
    mPolygon[5] = { 220.0f, 450.0f };
    mPolygon[6] = { 128.0f, 256.0f };
    mPolygon[7] = { 200.0f, 100.0f };

    LogAssert(
        mOrder(mPolygon),
        "The polygon must be counterclockwise.");

    mCHSP(mPolygon, mHull);
}

void ConvexHullSimplePolygonWindow2::Polygon1StartConvex()
{
    // Complicated topology, start with convex vertex.
    mPolygon.resize(16);
    mPolygon[0] = { 11.0f, 214.0f };
    mPolygon[1] = { 19.0f, 53.0f };
    mPolygon[2] = { 239.0f, 282.0f };
    mPolygon[3] = { 111.0f, 110.0f };
    mPolygon[4] = { 138.0f, 25.0f };
    mPolygon[5] = { 201.0f, 6.0f };
    mPolygon[6] = { 241.0f, 79.0f };
    mPolygon[7] = { 194.0f, 25.0f };
    mPolygon[8] = { 153.0f, 34.0f };
    mPolygon[9] = { 147.0f, 120.0f };
    mPolygon[10] = { 234.0f, 214.0f };
    mPolygon[11] = { 170.f, 53.0f };
    mPolygon[12] = { 295.0f, 180.0f };
    mPolygon[13] = { 315.0f, 439.0f };
    mPolygon[14] = { 166.0f, 232.0f };
    mPolygon[15] = { 190.0f, 365.0f };

    LogAssert(
        mOrder(mPolygon),
        "The polygon must be counterclockwise.");
    mCHSP(mPolygon, mHull);
}

void ConvexHullSimplePolygonWindow2::Polygon1StartReflex()
{
    // Complicated topology, start with reflex vertex.
    mPolygon.resize(16);
    mPolygon[0] = { 147.0f, 120.0f };
    mPolygon[1] = { 234.0f, 214.0f };
    mPolygon[2] = { 170.f, 53.0f };
    mPolygon[3] = { 295.0f, 180.0f };
    mPolygon[4] = { 315.0f, 439.0f };
    mPolygon[5] = { 166.0f, 232.0f };
    mPolygon[6] = { 190.0f, 365.0f };
    mPolygon[7] = { 11.0f, 214.0f };
    mPolygon[8] = { 19.0f, 53.0f };
    mPolygon[9] = { 239.0f, 282.0f };
    mPolygon[10] = { 111.0f, 110.0f };
    mPolygon[11] = { 138.0f, 25.0f };
    mPolygon[12] = { 201.0f, 6.0f };
    mPolygon[13] = { 241.0f, 79.0f };
    mPolygon[14] = { 194.0f, 25.0f };
    mPolygon[15] = { 153.0f, 34.0f };

    LogAssert(
        mOrder(mPolygon),
        "The polygon must be counterclockwise.");
    mCHSP(mPolygon, mHull);
}

