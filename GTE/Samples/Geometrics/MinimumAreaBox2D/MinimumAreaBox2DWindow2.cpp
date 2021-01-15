// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.12.05

#include "MinimumAreaBox2DWindow2.h"
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumAreaBox2.h>
#include <random>

MinimumAreaBox2DWindow2::MinimumAreaBox2DWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    // Compute the convex hull internally using arbitrary precision
    // arithmetic.
    typedef BSRational<UIntegerAP32> MABRational;
    MinimumAreaBox2<float, MABRational> mab2;

#if 1
    // Randomly generated points.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);
    Vector2<float> center{ 0.5f*mXSize, 0.5f*mYSize };
    Vector2<float> extent{ 0.25f*mXSize, 0.125f*mYSize };
    Vector2<float> axis[2] = { { 1.0f, 1.0f }, { -1.0f, 1.0f } };
    Normalize(axis[0]);
    Normalize(axis[1]);
    int const numVertices = 256;
    mVertices.resize(numVertices);
    for (auto& v : mVertices)
    {
        float angle = rnd(mte) * (float)GTE_C_TWO_PI;
        float radius = rnd(mte);
        // The casting is to avoid an incorrect compiler warning by g++
        // on Fedora 21 Linux.
        float u[2];
        u[0] = extent[0] * static_cast<float>(std::cos(angle));
        u[1] = extent[1] * static_cast<float>(std::sin(angle));
        v = center + radius * (u[0] * axis[0] + u[1] * axis[1]);
    }

    mMinimalBox = mab2(numVertices, &mVertices[0]);
#endif

#if 0
    std::string path = mEnvironment.GetPath("convexpolygon.txt");
    std::ifstream input(path);
    int numVertices;
    input >> numVertices;
    mVertices.resize(numVertices);
    std::vector<int> indices(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        input >> mVertices[i][0];
        input >> mVertices[i][1];
        indices[i] = i;
    }
    input.close();

#if 1
    mMinimalBox = mab2(numVertices, &mVertices[0], numVertices, &indices[0]);
#else
    mMinimalBox = mab2(numVertices, &mVertices[0], 0, nullptr);
#endif

#endif

#if 0
    // This data set leads to an intermediate bounding box for which point 0
    // supports two edges of the box and point 5 supports the other two
    // edges.  Point 0 and point 5 are at box corners that are diagonally
    // opposite.  The example led to fixing a bug in the update of the
    // extremes when the intermediate box is rotated.
    std::string path = mEnvironment.GetPath("projection.raw");
    std::ifstream input(path, std::ios::binary);
    std::vector<Vector2<double>> temp(9);
    mVertices.resize(9);
    input.read((char*)&temp[0], 9 * 2 * sizeof(double));
    input.close();
    for (int k = 0; k < 9; ++k)
    {
        mVertices[k][0] = 256.0f + 4096.0f * (float)temp[k][0];
        mVertices[k][1] = 256.0f + 4096.0f * (float)temp[k][1];
    }

    mMinimalBox = mab2(9, &mVertices[0], 0, nullptr);
#endif

    mHull = mab2.GetHull();
}

void MinimumAreaBox2DWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    // Draw the convex hull.
    int i0, i1, x0, y0, x1, y1;
    int numHull = static_cast<int>(mHull.size());
    for (i0 = numHull - 1, i1 = 0; i1 < numHull; i0 = i1++)
    {
        x0 = static_cast<int>(std::lrint(mVertices[mHull[i0]][0]));
        y0 = static_cast<int>(std::lrint(mVertices[mHull[i0]][1]));
        x1 = static_cast<int>(std::lrint(mVertices[mHull[i1]][0]));
        y1 = static_cast<int>(std::lrint(mVertices[mHull[i1]][1]));
        DrawLine(x0, y0, x1, y1, 0xFF0000FF);
    }

    // Draw the minimum area box.
    int const lookup[4][2] = { { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 } };
    std::array<Vector2<float>, 4> vertex;
    mMinimalBox.GetVertices(vertex);
    for (int i = 0; i < 4; ++i)
    {
        x0 = static_cast<int>(std::lrint(vertex[lookup[i][0]][0]));
        y0 = static_cast<int>(std::lrint(vertex[lookup[i][0]][1]));
        x1 = static_cast<int>(std::lrint(vertex[lookup[i][1]][0]));
        y1 = static_cast<int>(std::lrint(vertex[lookup[i][1]][1]));
        DrawLine(x0, y0, x1, y1, 0xFFFF0000);
    }

    // Draw the input points.
    for (auto const& v : mVertices)
    {
        int x = static_cast<int>(std::lrint(v[0]));
        int y = static_cast<int>(std::lrint(v[1]));
        DrawThickPixel(x, y, 1, 0xFF808080);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MinimumAreaBox2DWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Geometrics/MinimumAreaBox2D/Data/");

    if (mEnvironment.GetPath("convexpolygon.txt") == "")
    {
        LogError("Cannot find file convexpolygon.txt");
        return false;
    }

    if (mEnvironment.GetPath("projection.raw") == "")
    {
        LogError("Cannot find file projection.raw");
        return false;
    }

    return true;
}
