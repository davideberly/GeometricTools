// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.8.2021.03.26

#include "IncrementalDelaunay2Window2.h"
#include <random>

IncrementalDelaunay2Window2::IncrementalDelaunay2Window2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(static_cast<float>(mXSize)),
    mCurrent(0)
{
    mDoFlip = true;

    // Generate random points and triangulate.
    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(0.125f, 0.875f);

    mInputs.resize(32);
    for (size_t i = 0; i < mInputs.size(); ++i)
    {
        mInputs[i][0] = mSize * urd(dre);
        mInputs[i][1] = mSize * urd(dre);
    }

    mDelaunay = std::make_unique<IncrementalDelaunay2<float>>(0.0f, 0.0f, mSize, mSize);
    for (size_t i = 0; i < mInputs.size(); ++i)
    {
        mDelaunay->Insert(mInputs[i]);
    }

    mDelaunay->GetTriangulation(mVertices, mTriangles);
    mVertices[0] = { 0.0f, 0.0f };
    mVertices[1] = { mSize, 0.0f };
    mVertices[2] = { 0.0f, mSize };
    OnDisplay();
}

void IncrementalDelaunay2Window2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    uint32_t const gray = 0xFF808080;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const green = 0xFF00FF00;
    int x0, y0, x1, y1, x2, y2;
    Vector2<float> v0, v1, v2;

    // Draw the triangle mesh.
    std::set<Vector2<float>> used;
    for (size_t t = 0; t < mTriangles.size(); ++t)
    {
        auto const& tri = mTriangles[t];

        v0 = mVertices[tri[0]];
        x0 = static_cast<int32_t>(v0[0] + 0.5f);
        y0 = static_cast<int32_t>(v0[1] + 0.5f);

        v1 = mVertices[tri[1]];
        x1 = static_cast<int32_t>(v1[0] + 0.5f);
        y1 = static_cast<int32_t>(v1[1] + 0.5f);

        v2 = mVertices[tri[2]];
        x2 = static_cast<int32_t>(v2[0] + 0.5f);
        y2 = static_cast<int32_t>(v2[1] + 0.5f);

        if (tri[0] >= 3 && tri[1] >= 3)
        {
            DrawLine(x0, y0, x1, y1, gray);
        }
        else
        {
            if ((tri[0] < 3 && tri[1] >= 3) || (tri[0] >= 3 && tri[1] < 3))
            {
                DrawLine(x0, y0, x1, y1, green);
            }
        }

        if (tri[1] >= 3 && tri[2] >= 3)
        {
            DrawLine(x1, y1, x2, y2, gray);
        }
        else
        {
            if ((tri[1] < 3 && tri[2] >= 3) || (tri[1] >= 3 && tri[2] < 3))
            {
                DrawLine(x1, y1, x2, y2, green);
            }
        }

        if (tri[2] >= 3 && tri[0] >= 3)
        {
            DrawLine(x2, y2, x0, y0, gray);
        }
        else
        {
            if ((tri[2] < 3 && tri[0] >= 3) || (tri[2] >= 3 && tri[0] < 3))
            {
                DrawLine(x2, y2, x0, y0, green);
            }
        }

        used.insert(v0);
        used.insert(v1);
        used.insert(v2);
    }

    // Draw the supertriangle boundaries.
    v0 = mVertices[0];
    x0 = static_cast<int32_t>(v0[0] + 0.5f);
    y0 = static_cast<int32_t>(v0[1] + 0.5f);
    v1 = mVertices[1];
    x1 = static_cast<int32_t>(v1[0] + 0.5f);
    y1 = static_cast<int32_t>(v1[1] + 0.5f);
    v2 = mVertices[2];
    x2 = static_cast<int32_t>(v2[0] + 0.5f);
    y2 = static_cast<int32_t>(v2[1] + 0.5f);
    DrawLine(x0, y0, x1, y1, green);
    DrawLine(x1, y1, x2, y2, green);
    DrawLine(x2, y2, x0, y0, green);

    // Draw the vertices.
    for (auto const& v : used)
    {
        x0 = static_cast<int32_t>(v[0] + 0.5f);
        y0 = static_cast<int32_t>(v[1] + 0.5f);
        DrawThickPixel(x0, y0, 2, blue);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool IncrementalDelaunay2Window2::OnMouseClick(int button, int state,
    int x, int y, unsigned int modifiers)
{
    Vector2<float> position
    {
        static_cast<float>(x),
        mSize - 1.0f - static_cast<float>(y)
    };

    if (button == MOUSE_LEFT && state == MOUSE_DOWN)
    {
        if (modifiers & MODIFIER_SHIFT)
        {
            // Remove a point from the triangulation.
            float minSqrLength = std::numeric_limits<float>::max();
            size_t iMin = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < mVertices.size(); ++i)
            {
                Vector2<float> diff = position - mVertices[i];
                float sqrLength = Dot(diff, diff);
                if (sqrLength < minSqrLength)
                {
                    minSqrLength = sqrLength;
                    iMin = i;
                }
            }
            mDelaunay->Remove(mVertices[iMin]);
        }
        else
        {
            // Insert a point into the triangulation.
            mDelaunay->Insert(position);
        }
        mDelaunay->GetTriangulation(mVertices, mTriangles);
        mVertices[0] = { 0.0f, 0.0f };
        mVertices[1] = { mSize, 0.0f };
        mVertices[2] = { 0.0f, mSize };
        OnDisplay();
    }

    return true;
}
