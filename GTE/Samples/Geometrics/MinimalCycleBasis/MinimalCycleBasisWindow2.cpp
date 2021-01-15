// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "MinimalCycleBasisWindow2.h"
#include <Mathematics/IsPlanarGraph.h>

MinimalCycleBasisWindow2::MinimalCycleBasisWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mDrawRawData(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mDoFlip = true;

    // Possible inputs are "SimpleGraphN.txt", where N is in {0,1,2,3,4,5}.
    std::ifstream input(mEnvironment.GetPath("SimpleGraph0.txt"));
    int numPositions;
    input >> numPositions;
    mPositions.resize(numPositions);
    mFPositions.resize(numPositions);
    std::array<float, 2> vmin, vmax;
    vmin[0] = std::numeric_limits<float>::max();
    vmin[1] = std::numeric_limits<float>::max();
    vmax[0] = -std::numeric_limits<float>::max();
    vmax[1] = -std::numeric_limits<float>::max();
    for (int i = 0; i < numPositions; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            float value;
            input >> value;
            mPositions[i][j] = value;
            mFPositions[i][j] = value;
            if (value < vmin[j])
            {
                vmin[j] = value;
            }
            if (value > vmax[j])
            {
                vmax[j] = value;
            }
        }
    }
    int numEdges;
    input >> numEdges;
    mEdges.resize(numEdges);
    for (int i = 0; i < numEdges; ++i)
    {
        input >> mEdges[i][0];
        input >> mEdges[i][1];
    }
    input.close();

    IsPlanarGraph<Rational> isPlanarGraph;
    int result;
    result = isPlanarGraph(mPositions, mEdges);
    if (result != IsPlanarGraph<Rational>::IPG_IS_PLANAR_GRAPH)
    {
        parameters.created = false;
        return;
    }

    // Compute coefficients for mapping the graph bounding box to screen
    // space while preserving the aspect ratio.
    float ratioW = static_cast<float>(mXSize) / (vmax[0] - vmin[0]);
    float ratioH = static_cast<float>(mYSize) / (vmax[1] - vmin[1]);
    float vmult;
    if (ratioW <= ratioH)
    {
        vmult = static_cast<float>(mXSize - 1) / (vmax[0] - vmin[0]);
    }
    else
    {
        vmult = static_cast<float>(mYSize - 1) / (vmax[1] - vmin[1]);
    }
    mSPositions.resize(numPositions);
    for (int i = 0; i < numPositions; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            mSPositions[i][j] = static_cast<int>(vmult * (mFPositions[i][j] - vmin[j]));
        }
    }

    MinimalCycleBasis<Rational> mcb(mPositions, mEdges, mForest);
}

void MinimalCycleBasisWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    // Draw the edges.
    if (mDrawRawData)
    {
        for (auto edge : mEdges)
        {
            int x0 = mSPositions[edge[0]][0];
            int y0 = mSPositions[edge[0]][1];
            int x1 = mSPositions[edge[1]][0];
            int y1 = mSPositions[edge[1]][1];
            DrawLine(x0, y0, x1, y1, 0xFFFF0000);
        }
    }
    else
    {
        for (auto const& tree : mForest)
        {
            DrawTree(tree);
        }
    }

    if (mDrawRawData)
    {
        // Draw the input points.
        for (auto p : mSPositions)
        {
            int x = p[0];
            int y = p[1];
            DrawThickPixel(x, y, 0, 0xFF000000);
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool MinimalCycleBasisWindow2::OnCharPress(unsigned char key, int x, int y)
{
    if (key == 'c' || key == 'C')
    {
        mDrawRawData = !mDrawRawData;
        OnDisplay();
        return true;
    }
    return Window2::OnCharPress(key, x, y);
}

bool MinimalCycleBasisWindow2::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path != "")
    {
        mEnvironment.Insert(path + "/Samples/Geometrics/MinimalCycleBasis/Data/");
        for (int i = 0; i < 6; ++i)
        {
            std::string filename = "SimpleGraph" + std::to_string(i) + ".txt";
            if (mEnvironment.GetPath(filename) == "")
            {
                LogError("Cannot find input file " + filename);
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void MinimalCycleBasisWindow2::DrawTree(std::shared_ptr<MinimalCycleBasis<Rational>::Tree> const& tree)
{
    if (tree->cycle.size() > 0)
    {
        for (size_t i = 0; i + 1 < tree->cycle.size(); ++i)
        {
            int x0 = mSPositions[tree->cycle[i]][0];
            int y0 = mSPositions[tree->cycle[i]][1];
            int x1 = mSPositions[tree->cycle[i + 1]][0];
            int y1 = mSPositions[tree->cycle[i + 1]][1];
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
    }

    for (auto const& child : tree->children)
    {
        DrawTree(child);
    }
}
