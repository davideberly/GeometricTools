// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.06.08

#include "IncrementalDelaunay2Window2.h"
#include <random>

IncrementalDelaunay2Window2::IncrementalDelaunay2Window2(Parameters& parameters)
    :
    Window2(parameters),
    mSize(static_cast<float>(mXSize)),
    mDelaunay(0.0f, 0.0f, mSize - 1.0f, mSize - 1.0f),
    mInputs{},
    mVertices{},
    mTriangles{},
    mInfo{},
    mContainingTriangle(mDelaunay.invalid),
    mMessage("")
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

    for (size_t i = 0; i < mInputs.size(); ++i)
    {
        mDelaunay.Insert(mInputs[i]);
    }

    mDelaunay.GetTriangulation(mVertices, mTriangles);
    mVertices[0] = { 0.0f, 0.0f };
    mVertices[1] = { mSize, 0.0f };
    mVertices[2] = { 0.0f, mSize };

    OnDisplay();
}

void IncrementalDelaunay2Window2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const gray = 0xFF808080;
    uint32_t const blue = 0xFFFF0000;
    uint32_t const red = 0xFF0000FF;
    uint32_t const cyan = 0xFFFFFF00;
    uint32_t const rose = 0xFFC9AEFF;
    uint32_t const lime = 0xFF1DE6B5;
    uint32_t const turquoise = 0xFFEAD999;
    uint32_t const lavender = 0xFFE7BFC8;
    std::array<uint32_t, 3> const adjColor = { lime, turquoise, lavender };
    ClearScreen(white);

    int32_t x0, y0, x1, y1, x2, y2;
    Vector2<float> v0, v1, v2;

    // Draw a solid triangle and adjacents when requested.
    if (mContainingTriangle != mDelaunay.invalid)
    {
        // Draw the selected triangle.
        std::array<size_t, 3> tri{};
        mDelaunay.GetTriangle(mContainingTriangle, tri);
        v0 = mVertices[tri[0]];
        x0 = static_cast<int32_t>(v0[0] + 0.5f);
        y0 = static_cast<int32_t>(v0[1] + 0.5f);
        v1 = mVertices[tri[1]];
        x1 = static_cast<int32_t>(v1[0] + 0.5f);
        y1 = static_cast<int32_t>(v1[1] + 0.5f);
        v2 = mVertices[tri[2]];
        x2 = static_cast<int32_t>(v2[0] + 0.5f);
        y2 = static_cast<int32_t>(v2[1] + 0.5f);
        DrawLine(x0, y0, x1, y1, gray);
        DrawLine(x1, y1, x2, y2, gray);
        DrawLine(x2, y2, x0, y0, gray);
        v0 = (v0 + v1 + v2) / 3.0f;
        x0 = static_cast<int32_t>(v0[0] + 0.5f);
        y0 = static_cast<int32_t>(v0[1] + 0.5f);
        DrawFloodFill4(x0, y0, rose, white);

        // Draw the adjacent triangles.
        std::array<size_t, 3> adj;
        mDelaunay.GetAdjacent(mContainingTriangle, adj);
        for (size_t i = 0; i < 3; ++i)
        {
            if (adj[i] != mDelaunay.invalid)
            {
                mDelaunay.GetTriangle(adj[i], tri);
                v0 = mVertices[tri[0]];
                x0 = static_cast<int32_t>(v0[0] + 0.5f);
                y0 = static_cast<int32_t>(v0[1] + 0.5f);
                v1 = mVertices[tri[1]];
                x1 = static_cast<int32_t>(v1[0] + 0.5f);
                y1 = static_cast<int32_t>(v1[1] + 0.5f);
                v2 = mVertices[tri[2]];
                x2 = static_cast<int32_t>(v2[0] + 0.5f);
                y2 = static_cast<int32_t>(v2[1] + 0.5f);
                DrawLine(x0, y0, x1, y1, gray);
                DrawLine(x1, y1, x2, y2, gray);
                DrawLine(x2, y2, x0, y0, gray);
                v0 = (v0 + v1 + v2) / 3.0f;
                x0 = static_cast<int32_t>(v0[0] + 0.5f);
                y0 = static_cast<int32_t>(v0[1] + 0.5f);
                DrawFloodFill4(x0, y0, adjColor[i], white);
            }
        }
    }

    // Draw the triangle mesh.
    std::set<Vector2<float>> used;
    for (size_t t = 0; t < mTriangles.size(); ++t)
    {
        auto const& tri = mTriangles[t];
        if (tri[0] >= 3 && tri[1] >= 3 && tri[2] >= 3)
        {
            v0 = mVertices[tri[0]];
            x0 = static_cast<int32_t>(v0[0] + 0.5f);
            y0 = static_cast<int32_t>(v0[1] + 0.5f);

            v1 = mVertices[tri[1]];
            x1 = static_cast<int32_t>(v1[0] + 0.5f);
            y1 = static_cast<int32_t>(v1[1] + 0.5f);

            v2 = mVertices[tri[2]];
            x2 = static_cast<int32_t>(v2[0] + 0.5f);
            y2 = static_cast<int32_t>(v2[1] + 0.5f);

            DrawLine(x0, y0, x1, y1, gray);
            DrawLine(x1, y1, x2, y2, gray);
            DrawLine(x2, y2, x0, y0, gray);

            used.insert(v0);
            used.insert(v1);
            used.insert(v2);
        }
    }

    // Draw only the Delaunay triangles (skip the triangles with at least
    // one supervertex).
    auto const& delaunayTriangles = mDelaunay.GetTriangles();
    for (size_t i = 0; i < delaunayTriangles.size(); ++i)
    {
        auto const& tri = delaunayTriangles[i];

        v0 = mVertices[tri[0]];
        x0 = static_cast<int32_t>(v0[0] + 0.5f);
        y0 = static_cast<int32_t>(v0[1] + 0.5f);

        v1 = mVertices[tri[1]];
        x1 = static_cast<int32_t>(v1[0] + 0.5f);
        y1 = static_cast<int32_t>(v1[1] + 0.5f);

        v2 = mVertices[tri[2]];
        x2 = static_cast<int32_t>(v2[0] + 0.5f);
        y2 = static_cast<int32_t>(v2[1] + 0.5f);

        DrawLine(x0, y0, x1, y1, red);
        DrawLine(x1, y1, x2, y2, red);
        DrawLine(x2, y2, x0, y0, red);
    }

    // Draw the convex hull of the Delaunay triangles.
    std::vector<size_t> hull;
    mDelaunay.GetHull(hull);
    v0 = mVertices[hull[0]];
    x0 = static_cast<int32_t>(v0[0] + 0.5f);
    y0 = static_cast<int32_t>(v0[1] + 0.5f);
    for (size_t i = 1; i < hull.size(); ++i)
    {
        v1 = mVertices[hull[i]];
        x1 = static_cast<int32_t>(v1[0] + 0.5f);
        y1 = static_cast<int32_t>(v1[1] + 0.5f);
        DrawLine(x0, y0, x1, y1, cyan);
        x0 = x1;
        y0 = y1;
    }
    v1 = mVertices[hull[0]];
    x1 = static_cast<int32_t>(v1[0] + 0.5f);
    y1 = static_cast<int32_t>(v1[1] + 0.5f);
    DrawLine(x0, y0, x1, y1, cyan);

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

void IncrementalDelaunay2Window2::DrawScreenOverlay()
{
    if (mMessage != "")
    {
        mEngine->Draw(8, 24, { 0.0f, 0.0f, 0.0f, 1.0f }, mMessage);
    }
}

bool IncrementalDelaunay2Window2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'c':
    case 'C':
        mInfo = IncrementalDelaunay2<float>::SearchInfo{};
        mContainingTriangle = mDelaunay.invalid;
        return true;

    case 'f':
    case 'F':
        if (mDelaunay.FinalizeTriangulation())
        {
            mDelaunay.GetTriangulation(mVertices, mTriangles);
            mVertices[0] = { 0.0f, 0.0f };
            mVertices[1] = { mSize, 0.0f };
            mVertices[2] = { 0.0f, mSize };
            OnDisplay();
        }
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

bool IncrementalDelaunay2Window2::OnMouseClick(int32_t button, int32_t state,
    int32_t x, int32_t y, uint32_t modifiers)
{
    Vector2<float> position
    {
        static_cast<float>(x),
        mSize - 1.0f - static_cast<float>(y)
    };

    if (state == MOUSE_DOWN)
    {
        if (button == MOUSE_LEFT)
        {
            size_t index = std::numeric_limits<size_t>::max();
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

                if (iMin >= 7)
                {
                    index = mDelaunay.Remove(mVertices[iMin]);
                }
            }
            else
            {
                // Insert a point into the triangulation.
                index = mDelaunay.Insert(position);
            }

            if (index != std::numeric_limits<size_t>::max())
            {
                mDelaunay.GetTriangulation(mVertices, mTriangles);
                mVertices[0] = { 0.0f, 0.0f };
                mVertices[1] = { mSize, 0.0f };
                mVertices[2] = { 0.0f, mSize };
            }
        }
        else if (button == MOUSE_RIGHT)
        {
            if (modifiers & MODIFIER_SHIFT)
            {
                mInfo.initialTriangle = mInfo.finalTriangle;
                mContainingTriangle = mDelaunay.GetContainingTriangle(position, mInfo);
            }
            else
            {
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
                if (iMin != std::numeric_limits<size_t>::max() &&
                    std::sqrt(minSqrLength) <= 8.0f)
                {
                    mMessage = "vertex " + std::to_string(iMin);
                }
                else
                {
                    mMessage = "";
                }
            }
        }
        OnDisplay();
    }

    return true;
}
