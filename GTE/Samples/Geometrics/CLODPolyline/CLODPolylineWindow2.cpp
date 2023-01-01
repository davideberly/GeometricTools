// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "CLODPolylineWindow2.h"
#include <random>

CLODPolylineWindow2::CLODPolylineWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(0.75f, 1.25f);
    int32_t numVertices = 16;
    std::vector<Vector3<float>> vertices(numVertices);
    for (int32_t i = 0; i < numVertices; ++i)
    {
        float angle = static_cast<float>(GTE_C_TWO_PI * i / numVertices);
        vertices[i] = urd(dre)* Vector3<float>{ std::cos(angle), std::sin(angle), 0.0f };
    }

    mPolyline = std::make_unique<CLODPolyline<3, float>>(vertices, true);
}

void CLODPolylineWindow2::OnDisplay()
{
    uint32_t white = 0xFFFFFFFF;
    uint32_t black = 0xFF000000;

    ClearScreen(white);

    int32_t numVertices = mPolyline->GetNumVertices();
    auto const& vertices = mPolyline->GetVertices();
    int32_t numEdges = mPolyline->GetNumEdges();
    auto const& edges = mPolyline->GetEdges();

    for (int32_t i = 0; i < numVertices; ++i)
    {
        int32_t x, y;
        Get(vertices[i], x, y);
        DrawThickPixel(x, y, 1, black);
    }

    for (int32_t i = 0; i < numEdges; ++i)
    {
        size_t twoI = 2 * static_cast<size_t>(i);
        int32_t x0, y0, x1, y1;
        Get(vertices[edges[twoI]], x0, y0);
        Get(vertices[edges[twoI + 1]], x1, y1);
        DrawLine(x0, y0, x1, y1, black);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool CLODPolylineWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    int32_t levelOfDetail;

    switch (key)
    {
    case '+':  // increase level of detail
    case '=':
        levelOfDetail = mPolyline->GetLevelOfDetail();
        if (levelOfDetail < mPolyline->GetMaxLevelOfDetail())
        {
            mPolyline->SetLevelOfDetail(levelOfDetail + 1);
            OnDisplay();
        }
        return true;
    case '-':  // decrease level of detail
    case '_':
        levelOfDetail = mPolyline->GetLevelOfDetail();
        if (levelOfDetail > mPolyline->GetMinLevelOfDetail())
        {
            mPolyline->SetLevelOfDetail(levelOfDetail - 1);
            OnDisplay();
        }
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}
