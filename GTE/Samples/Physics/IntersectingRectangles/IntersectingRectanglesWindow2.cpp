// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "IntersectingRectanglesWindow2.h"

IntersectingRectanglesWindow2::IntersectingRectanglesWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(static_cast<float>(mXSize)),
    mPerturb(-4.0f, 4.0f)
{
    std::uniform_real_distribution<float> rnd(0.125f * mSize, 0.875f * mSize);
    std::uniform_real_distribution<float> intrrnd(8.0f, 64.0f);
    for (int32_t i = 0; i < 16; ++i)
    {
        Vector2<float> min{ rnd(mMTE), rnd(mMTE) };
        Vector2<float> max{ min[0] + intrrnd(mMTE), min[1] + intrrnd(mMTE) };
        mRectangles.push_back(AlignedBox2<float>(min, max));
    }

    mManager = std::make_unique<RectangleManager<float>>(mRectangles);
    mLastIdle = mTimer.GetSeconds();
    OnDisplay();
}

void IntersectingRectanglesWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);
    DrawRectangles();
    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void IntersectingRectanglesWindow2::OnIdle()
{
    double currIdle = mTimer.GetSeconds();
    double diff = currIdle - mLastIdle;
    if (diff >= 1.0 / 30.0)
    {
        ModifyRectangles();
        OnDisplay();
        mLastIdle = currIdle;
    }
}

void IntersectingRectanglesWindow2::ModifyRectangles()
{
    int32_t i = 0;
    for (auto rectangle : mRectangles)
    {
        float dx = mPerturb(mMTE);
        if (0.0f <= rectangle.min[0] + dx && rectangle.max[0] + dx < mSize)
        {
            rectangle.min[0] += dx;
            rectangle.max[0] += dx;
        }

        float dy = mPerturb(mMTE);
        if (0.0f <= rectangle.min[1] + dy && rectangle.max[1] + dy < mSize)
        {
            rectangle.min[1] += dy;
            rectangle.max[1] += dy;
        }

        mManager->SetRectangle(i, rectangle);
        ++i;
    }

    mManager->Update();
}

void IntersectingRectanglesWindow2::DrawRectangles()
{
    uint32_t const gray = 0xFFC0C0C0, black = 0, red = 0xFF0000FF;
    int32_t xmin, xmax, ymin, ymax;

    for (auto rectangle : mRectangles)
    {
        xmin = static_cast<int32_t>(std::lrint(rectangle.min[0]));
        xmax = static_cast<int32_t>(std::lrint(rectangle.max[0]));
        ymin = static_cast<int32_t>(std::lrint(rectangle.min[1]));
        ymax = static_cast<int32_t>(std::lrint(rectangle.max[1]));
        DrawRectangle(xmin, ymin, xmax, ymax, gray, true);
        DrawRectangle(xmin, ymin, xmax, ymax, black, false);
    }

    FIQuery<float, AlignedBox2<float>, AlignedBox2<float>> query;
    for (auto const& overlap : mManager->GetOverlap())
    {
        int32_t i0 = overlap.V[0], i1 = overlap.V[1];
        auto result = query(mRectangles[i0], mRectangles[i1]);
        if (result.intersect)
        {
            xmin = static_cast<int32_t>(std::lrint(result.box.min[0]));
            xmax = static_cast<int32_t>(std::lrint(result.box.max[0]));
            ymin = static_cast<int32_t>(std::lrint(result.box.min[1]));
            ymax = static_cast<int32_t>(std::lrint(result.box.max[1]));
            DrawRectangle(xmin, ymin, xmax, ymax, red, true);
            DrawRectangle(xmin, ymin, xmax, ymax, black, false);
        }
    }
}
