// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.06.08

#include "IntersectTriangles2DWindow2.h"
#include <Mathematics/ContPointInPolygon2.h>

IntersectTriangles2DWindow2::IntersectTriangles2DWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mActive(0),
    mHasIntersection(false),
    mDoTIQuery(true)
{
    mTriangle[0].v[0] = { 260.0f, 260.0f };
    mTriangle[0].v[1] = { 388.0f, 260.0f };
    mTriangle[0].v[2] = { 420.0f, 400.0f };

    mTriangle[1].v[0] = { 252.0f, 252.0f };
    mTriangle[1].v[1] = { 152.0f, 248.0f };
    mTriangle[1].v[2] = { 200.0f, 100.0f };

    mDoFlip = true;
    DoQuery();
}

void IntersectTriangles2DWindow2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const redL = 0xFF0000FF;
    uint32_t const redD = 0xFF000080;
    uint32_t const blueL = 0xFFFF0000;
    uint32_t const blueD = 0xFF800000;
    uint32_t const yellowL = 0xFF00FFFF;
    uint32_t const yellowD = 0xFF0080FF;
    uint32_t const greenL = 0xFF00FF00;
    uint32_t const greenD = 0xFF008000;

    ClearScreen(white);

    if (mHasIntersection)
    {
        DrawTriangle(mTriangle[0].v, yellowL, yellowD);
        DrawTriangle(mTriangle[1].v, greenL, greenD);
        if (!mDoTIQuery && mIntersection.size() >= 3)
        {
            DrawIntersection();
        }
    }
    else
    {
        DrawTriangle(mTriangle[0].v, redL, redD);
        DrawTriangle(mTriangle[1].v, blueL, blueD);
        if (!mDoTIQuery && mIntersection.size() >= 3)
        {
            DrawIntersection();
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool IntersectTriangles2DWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    float const trnDelta = 1.0f;
    double const degrees = 1.0;
    float const rotDelta = static_cast<float>(GTE_C_DEG_TO_RAD * degrees);

    switch (key)
    {
    case '0':  // mTriangle[0] active for translation/rotation
        mActive = 0;
        return true;
    case '1':  // mTriangle[1] active for translation/rotation
        mActive = 1;
        return true;
    case 'x':  // translate active triangle in -x direction
        mTriangle[mActive].v[0][0] -= trnDelta;
        mTriangle[mActive].v[1][0] -= trnDelta;
        mTriangle[mActive].v[2][0] -= trnDelta;
        DoQuery();
        return true;
    case 'X':  // translate active triangle in +x direction
        mTriangle[mActive].v[0][0] += trnDelta;
        mTriangle[mActive].v[1][0] += trnDelta;
        mTriangle[mActive].v[2][0] += trnDelta;
        DoQuery();
        return true;
    case 'y':  // translate active triangle in -y direction
        mTriangle[mActive].v[0][1] -= trnDelta;
        mTriangle[mActive].v[1][1] -= trnDelta;
        mTriangle[mActive].v[2][1] -= trnDelta;
        DoQuery();
        return true;
    case 'Y':  // translate active triangle in +y direction
        mTriangle[mActive].v[0][1] += trnDelta;
        mTriangle[mActive].v[1][1] += trnDelta;
        mTriangle[mActive].v[2][1] += trnDelta;
        DoQuery();
        return true;
    case 'r':  // rotate active triangle clockwise
    {
        Vector2<float> C{ 0.0f, 0.0f };
        for (int32_t i = 0; i < 3; ++i)
        {
            C += mTriangle[mActive].v[i];
        }
        C /= 3.0f;

        float cs = std::cos(-rotDelta), sn = std::sin(-rotDelta);
        for (int32_t i = 0; i < 3; ++i)
        {
            Vector2<float> u = mTriangle[mActive].v[i] - C;
            mTriangle[mActive].v[i] =
            {
                C[0] + cs * u[0] - sn * u[1],
                C[1] + sn * u[0] + cs * u[1]
            };
        }
        DoQuery();
        return true;
    }
    case 'R':  // rotate active triangle counterclockwise
    {
        Vector2<float> C{ 0.0f, 0.0f };
        for (int32_t i = 0; i < 3; ++i)
        {
            C += mTriangle[mActive].v[i];
        }
        C /= 3.0f;

        float cs = std::cos(+rotDelta), sn = std::sin(+rotDelta);
        for (int32_t i = 0; i < 3; ++i)
        {
            Vector2<float> u = mTriangle[mActive].v[i] - C;
            mTriangle[mActive].v[i] =
            {
                C[0] + cs * u[0] - sn * u[1],
                C[1] + sn * u[0] + cs * u[1]
            };
        }
        DoQuery();
        return true;
    }
    case ' ':
        // Support debugging by allowing a DoQuery to occur for the
        // current triangle configuration.
        DoQuery();
        return true;
    case 'f':
    case 'F':
        mDoTIQuery = !mDoTIQuery;
        DoQuery();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

bool IntersectTriangles2DWindow2::OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers)
{
    return Window2::OnMouseClick(button, state, x, y, modifiers);
}

bool IntersectTriangles2DWindow2::OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers)
{
    return Window2::OnMouseMotion(button, x, y, modifiers);
}

void IntersectTriangles2DWindow2::DrawTriangle(std::array<Vector2<float>, 3> const& vertex,
    uint32_t colorL, uint32_t colorD)
{
    Vector2<float> vmin{}, vmax{};
    ComputeExtremes(3, vertex.data(), vmin, vmax);
    int32_t xmin = static_cast<int32_t>(std::floor(vmin[0]));
    int32_t ymin = static_cast<int32_t>(std::floor(vmin[1]));
    int32_t xmax = static_cast<int32_t>(std::ceil(vmax[0]));
    int32_t ymax = static_cast<int32_t>(std::ceil(vmax[1]));

    PointInPolygon2<float> pip(3, vertex.data());
    Vector2<float> test;
    for (int32_t y = ymin; y <= ymax; ++y)
    {
        test[1] = (float)y;
        for (int32_t x = xmin; x <= xmax; ++x)
        {
            test[0] = (float)x;
            if (pip.ContainsConvexOrderN(test))
            {
                SetPixel(x, y, colorL);
            }
        }
    }

    int32_t x0 = static_cast<int32_t>(vertex[0][0]);
    int32_t y0 = static_cast<int32_t>(vertex[0][1]);
    int32_t x1 = static_cast<int32_t>(vertex[1][0]);
    int32_t y1 = static_cast<int32_t>(vertex[1][1]);
    int32_t x2 = static_cast<int32_t>(vertex[2][0]);
    int32_t y2 = static_cast<int32_t>(vertex[2][1]);
    DrawLine(x0, y0, x1, y1, colorD);
    DrawLine(x1, y1, x2, y2, colorD);
    DrawLine(x2, y2, x0, y0, colorD);
}

void IntersectTriangles2DWindow2::DrawIntersection()
{
    uint32_t const black = 0xFF000000;
    uint32_t const gray = 0xFF7F7F7F;

    Vector2<float> vmin{}, vmax{};
    ComputeExtremes(static_cast<int32_t>(mIntersection.size()), mIntersection.data(),
        vmin, vmax);
    int32_t xmin = static_cast<int32_t>(std::floor(vmin[0]));
    int32_t ymin = static_cast<int32_t>(std::floor(vmin[1]));
    int32_t xmax = static_cast<int32_t>(std::ceil(vmax[0]));
    int32_t ymax = static_cast<int32_t>(std::ceil(vmax[1]));

    PointInPolygon2<float> pip(static_cast<int32_t>(mIntersection.size()), mIntersection.data());
    Vector2<float> test;
    for (int32_t y = ymin; y <= ymax; ++y)
    {
        test[1] = static_cast<float>(y);
        for (int32_t x = xmin; x <= xmax; ++x)
        {
            test[0] = static_cast<float>(x);
            if (pip.ContainsConvexOrderN(test))
            {
                SetPixel(x, y, gray);
            }
        }
    }

    int32_t x0, y0, x1, y1;
    x0 = static_cast<int32_t>(mIntersection[0][0]);
    y0 = static_cast<int32_t>(mIntersection[0][1]);
    for (size_t i = 1; i < mIntersection.size(); ++i)
    {
        x1 = static_cast<int32_t>(mIntersection[i][0]);
        y1 = static_cast<int32_t>(mIntersection[i][1]);
        DrawLine(x0, y0, x1, y1, black);
        x0 = x1;
        y0 = y1;
    }
    x1 = static_cast<int32_t>(mIntersection[0][0]);
    y1 = static_cast<int32_t>(mIntersection[0][1]);
    DrawLine(x0, y0, x1, y1, black);
}

void IntersectTriangles2DWindow2::DoQuery()
{
    if (mDoTIQuery)
    {
        mHasIntersection = mTIQuery(mTriangle[0], mTriangle[1]).intersect;
    }
    else
    {
        mIntersection = mFIQuery(mTriangle[0], mTriangle[1]).intersection;
        mHasIntersection = (mIntersection.size() > 0);
    }

    OnDisplay();
}
