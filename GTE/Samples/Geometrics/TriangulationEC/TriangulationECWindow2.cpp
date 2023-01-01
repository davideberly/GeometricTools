// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TriangulationECWindow2.h"

TriangulationECWindow2::TriangulationECWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mTree(nullptr),
    mExample(0)
{
    mClampToWindow = false;
    mDoFlip = true;
    UnindexedSimplePolygon();
}

void TriangulationECWindow2::OnDisplay()
{
    ClearScreen(0xFFFFFFFF);

    int32_t i, i0, i1, numPositions, x0, y0, x1, y1;

    // Draw the polygon edges.
    switch (mExample)
    {
    case 0:
        numPositions = static_cast<int32_t>(mPositions.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[i0][0]);
            y0 = static_cast<int32_t>(mPositions[i0][1]);
            x1 = static_cast<int32_t>(mPositions[i1][0]);
            y1 = static_cast<int32_t>(mPositions[i1][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
        break;
    case 1:
        numPositions = static_cast<int32_t>(mOuter.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mOuter[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mOuter[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mOuter[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mOuter[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
        break;
    case 2:
        numPositions = static_cast<int32_t>(mOuter.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mOuter[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mOuter[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mOuter[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mOuter[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner0.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner0[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner0[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner0[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner0[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
        break;

    case 3:
        numPositions = static_cast<int32_t>(mOuter.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mOuter[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mOuter[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mOuter[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mOuter[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner0.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner0[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner0[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner0[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner0[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner1.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner1[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner1[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner1[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner1[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
        break;

    case 4:
    {
        std::queue<std::shared_ptr<PolygonTree>> treeQueue;
        treeQueue.push(mTree);
        while (treeQueue.size() > 0)
        {
            std::shared_ptr<PolygonTree> tree = treeQueue.front();
            treeQueue.pop();
            numPositions = static_cast<int32_t>(tree->polygon.size());
            int32_t const* indices = tree->polygon.data();
            for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
            {
                x0 = static_cast<int32_t>(mPositions[indices[i0]][0]);
                y0 = static_cast<int32_t>(mPositions[indices[i0]][1]);
                x1 = static_cast<int32_t>(mPositions[indices[i1]][0]);
                y1 = static_cast<int32_t>(mPositions[indices[i1]][1]);
                DrawLine(x0, y0, x1, y1, 0xFF000000);
            }

            for (i = 0; i < (int32_t)tree->child.size(); ++i)
            {
                treeQueue.push(tree->child[i]);
            }
        }
        break;
    }

    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        numPositions = static_cast<int32_t>(mOuter.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mOuter[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mOuter[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mOuter[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mOuter[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner0.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner0[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner0[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner0[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner0[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner1.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner1[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner1[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner1[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner1[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }

        numPositions = static_cast<int32_t>(mInner2.size());
        for (i0 = numPositions - 1, i1 = 0; i1 < numPositions; i0 = i1++)
        {
            x0 = static_cast<int32_t>(mPositions[mInner2[i0]][0]);
            y0 = static_cast<int32_t>(mPositions[mInner2[i0]][1]);
            x1 = static_cast<int32_t>(mPositions[mInner2[i1]][0]);
            y1 = static_cast<int32_t>(mPositions[mInner2[i1]][1]);
            DrawLine(x0, y0, x1, y1, 0xFF000000);
        }
        break;
    }

    // Flood fill the polygon inside.
    for (auto const& seed : mFillSeeds)
    {
        DrawFloodFill4(static_cast<int32_t>(seed[0]), static_cast<int32_t>(seed[1]), 0xFFFF0000, 0xFFFFFFFF);
    }

    // Draw the triangulation edges.
    for (auto const& tri : mTriangles)
    {
        int32_t v0 = tri[0];
        int32_t v1 = tri[1];
        int32_t v2 = tri[2];

        x0 = static_cast<int32_t>(mPositions[v0][0]);
        y0 = static_cast<int32_t>(mPositions[v0][1]);
        x1 = static_cast<int32_t>(mPositions[v1][0]);
        y1 = static_cast<int32_t>(mPositions[v1][1]);
        DrawLine(x0, y0, x1, y1, 0xFF000000);

        x0 = static_cast<int32_t>(mPositions[v1][0]);
        y0 = static_cast<int32_t>(mPositions[v1][1]);
        x1 = static_cast<int32_t>(mPositions[v2][0]);
        y1 = static_cast<int32_t>(mPositions[v2][1]);
        DrawLine(x0, y0, x1, y1, 0xFF000000);

        x0 = static_cast<int32_t>(mPositions[v2][0]);
        y0 = static_cast<int32_t>(mPositions[v2][1]);
        x1 = static_cast<int32_t>(mPositions[v0][0]);
        y1 = static_cast<int32_t>(mPositions[v0][1]);
        DrawLine(x0, y0, x1, y1, 0xFF000000);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool TriangulationECWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        UnindexedSimplePolygon();
        OnDisplay();
        return true;
    case '1':
        IndexedSimplePolygon();
        OnDisplay();
        return true;
    case '2':
        OneNestedPolygon();
        OnDisplay();
        return true;
    case '3':
        TwoNestedPolygons();
        OnDisplay();
        return true;
    case '4':
        TreeOfNestedPolygons();
        OnDisplay();
        return true;
    case '5':
        FourBoxesThreeNested(0, 1, 2);
        OnDisplay();
        return true;
    case '6':
        FourBoxesThreeNested(0, 2, 1);
        OnDisplay();
        return true;
    case '7':
        FourBoxesThreeNested(1, 0, 2);
        OnDisplay();
        return true;
    case '8':
        FourBoxesThreeNested(1, 2, 0);
        OnDisplay();
        return true;
    case '9':
        FourBoxesThreeNested(2, 0, 1);
        OnDisplay();
        return true;
    case 'a':
    case 'A':
        FourBoxesThreeNested(2, 1, 0);
        OnDisplay();
        return true;
    }

    return Window2::OnCharPress(key, x, y);
}

void TriangulationECWindow2::ClearAll()
{
    mPositions.clear();
    mOuter.clear();
    mInner0.clear();
    mInner1.clear();
    mInner2.clear();
    mTree = nullptr;
    mFillSeeds.clear();
    mTriangles.clear();
}

void TriangulationECWindow2::UnindexedSimplePolygon()
{
    ClearAll();
    mExample = 0;

    mPositions =
    {
        { 58.0f, 278.0f },
        { 156.0f, 198.0f },
        { 250.0f, 282.0f },
        { 328.0f, 232.0f },
        { 402.0f, 336.0f },
        { 314.0f, 326.0f },
        { 274.0f, 400.0f },
        { 196.0f, 268.0f },
        { 104.0f, 292.0f },
        { 110.0f, 382.0f }
    };

    mFillSeeds.push_back({ 132.0f, 256.0f });

    Triangulator triangulator(mPositions);
    triangulator();
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow2::IndexedSimplePolygon()
{
    ClearAll();
    mExample = 1;

    mPositions =
    {
        { 58.0f, 278.0f },
        { 0.0f, 0.0f },
        { 156.0f, 198.0f },
        { 0.0f, 0.0f },
        { 250.0f, 282.0f },
        { 0.0f, 0.0f },
        { 328.0f, 232.0f },
        { 0.0f, 0.0f },
        { 402.0f, 336.0f },
        { 0.0f, 0.0f },
        { 314.0f, 326.0f },
        { 0.0f, 0.0f },
        { 274.0f, 400.0f },
        { 0.0f, 0.0f },
        { 196.0f, 268.0f },
        { 0.0f, 0.0f },
        { 104.0f, 292.0f },
        { 0.0f, 0.0f },
        { 110.0f, 382.0f },
        { 0.0f, 0.0f }
    };

    mOuter = { 0, 2, 4, 6, 8, 10, 12, 14, 16, 18 };
    mFillSeeds.push_back({ 132.0f, 256 });

    Triangulator triangulator(mPositions);
    triangulator(mOuter);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow2::OneNestedPolygon()
{
    // Polygon with one hole.  The top and bottom vertices of the outer
    // polygon are on the line containing the left edge of the inner polygon.
    // This example tests how the collinearity detection works when
    // identifying ears.

    ClearAll();
    mExample = 2;

    mPositions =
    {
        { 128.0f, 256.0f },
        { 256.0f, 128.0f },
        { 384.0f, 256.0f },
        { 256.0f, 384.0f },
        { 320.0f, 256.0f },
        { 256.0f, 192.0f },
        { 256.0f, 320.0f }
    };

    mOuter = { 0, 1, 2, 3 };
    mInner0 = { 4, 5, 6 };
    mFillSeeds.push_back({ 132.0f, 256.0f });

    Triangulator triangulator(mPositions);
    triangulator(mOuter, mInner0);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow2::TwoNestedPolygons()
{
    ClearAll();
    mExample = 3;

    mPositions =
    {
        { 58.0f, 278.0f },
        { 156.0f, 198.0f },
        { 250.0f, 282.0f },
        { 328.0f, 232.0f },
        { 402.0f, 336.0f },
        { 314.0f, 326.0f },
        { 274.0f, 400.0f },
        { 196.0f, 268.0f },
        { 104.0f, 292.0f },
        { 110.0f, 382.0f },
        { 280.0f, 336.0f },
        { 327.0f, 283.0f },
        { 240.0f, 317.0f },
        { 106.0f, 256.0f },
        { 152.0f, 255.0f },
        { 201.0f, 249.0f }
    };

    mOuter = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    mInner0 = { 11, 12, 10 };
    mInner1 = { 13, 14, 15 };
    mFillSeeds.push_back({ 62.0f, 278.0f });

    Triangulator triangulator(mPositions);
    std::vector<Triangulator::Polygon> inners{ mInner0, mInner1 };
    triangulator(mOuter, inners);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow2::TreeOfNestedPolygons()
{
    ClearAll();
    mExample = 4;

    mPositions =
    {
        { 204.0f, 30.0f },
        { 466.0f, 174.0f },
        { 368.0f, 496.0f },
        { 66.0f, 464.0f },
        { 28.0f, 256.0f },
        { 274.0f, 84.0f },
        { 186.0f, 82.0f },
        { 274.0f, 158.0f },
        { 292.0f, 132.0f },
        { 322.0f, 426.0f },
        { 426.0f, 226.0f },
        { 216.0f, 134.0f },
        { 72.0f, 306.0f },
        { 178.0f, 440.0f },
        { 266.0f, 372.0f },
        { 294.0f, 474.0f },
        { 354.0f, 474.0f },
        { 368.0f, 404.0f },
        { 318.0f, 450.0f },
        { 172.0f, 226.0f },
        { 230.0f, 236.0f },
        { 196.0f, 268.0f },
        { 218.0f, 306.0f },
        { 136.0f, 266.0f },
        { 136.0f, 312.0f },
        { 230.0f, 350.0f },
        { 216.0f, 388.0f },
        { 160.0f, 384.0f },
        { 326.0f, 216.0f },
        { 370.0f, 216.0f },
        { 344.0f, 352.0f },
        { 158.0f, 340.0f },
        { 158.0f, 358.0f },
        { 176.0f, 358.0f },
        { 176.0f, 340.0f },
        { 192.0f, 358.0f },
        { 192.0f, 374.0f },
        { 206.0f, 374.0f },
        { 206.0f, 358.0f },
        { 338.0f, 242.0f },
        { 338.0f, 262.0f },
        { 356.0f, 262.0f },
        { 356.0f, 242.0f }
    };

    // outer0 polygon
    mTree = std::make_shared<PolygonTree>();
    mTree->polygon = { 0, 1, 2, 3, 4 };
    mFillSeeds.push_back({ 164.0f, 138.0f });

    // inner0 polygon
    auto inner0 = std::make_shared<PolygonTree>();
    inner0->polygon = { 5, 6, 7 };
    mTree->child.push_back(inner0);

    // inner1 polygon
    auto inner1 = std::make_shared<PolygonTree>();
    inner1->polygon = { 8, 9, 10 };
    mTree->child.push_back(inner1);

    // inner2 polygon
    auto inner2 = std::make_shared<PolygonTree>();
    inner2->polygon = { 11, 12, 13, 14, 15, 16, 17, 18 };
    mTree->child.push_back(inner2);

    // outer1 polygon (contained in inner2)
    auto outer1 = std::make_shared<PolygonTree>();
    outer1->polygon = { 19, 20, 21, 22, 23 };
    inner2->child.push_back(outer1);
    mFillSeeds.push_back({ 184.0f, 248.0f });

    // outer2 polygon (contained in inner2)
    auto outer2 = std::make_shared<PolygonTree>();
    outer2->polygon = { 24, 25, 26, 27 };
    inner2->child.push_back(outer2);
    mFillSeeds.push_back({ 218.0f, 358.0f });

    // outer3 polygon (contained in inner1)
    auto outer3 = std::make_shared<PolygonTree>();
    outer3->polygon = { 28, 29, 30 };
    inner1->child.push_back(outer3);
    mFillSeeds.push_back({ 344.0f, 278.0f });

    // inner3 polygon (contained in outer2)
    auto inner3 = std::make_shared<PolygonTree>();
    inner3->polygon = { 31, 32, 33, 34 };
    outer2->child.push_back(inner3);

    // inner4 polygon (contained in outer2)
    auto inner4 = std::make_shared<PolygonTree>();
    inner4->polygon = { 35, 36, 37, 38 };
    outer2->child.push_back(inner4);

    // inner5 polygon (contained in outer3)
    auto inner5 = std::make_shared<PolygonTree>();
    inner5->polygon = { 39, 40, 41, 42 };
    outer3->child.push_back(inner5);

    Triangulator triangulator(mPositions);
    triangulator(mTree);
    mTriangles = triangulator.GetTriangles();
}

void TriangulationECWindow2::FourBoxesThreeNested(int32_t i0, int32_t i1, int32_t i2)
{
    ClearAll();
    mExample = 5;

    mPositions =
    {
        { 64.0f, 16.0f },
        { 448.0f, 16.0f },
        { 448.0f, 496.0f },
        { 64.0f, 496.0f },
        { 320.0f, 32.0f },
        { 320.0f, 240.0f },
        { 384.0f, 240.0f },
        { 384.0f, 32.0f },
        { 320.0f, 272.0f },
        { 320.0f, 480.0f },
        { 384.0f, 480.0f },
        { 384.0f, 272.0f },
        { 128.0f, 272.0f },
        { 128.0f, 480.0f },
        { 192.0f, 480.0f },
        { 192.0f, 272.0f }
    };

    mOuter = { 0, 1, 2, 3 };
    mInner0 = { 4, 5, 6, 7 };
    mInner1 = { 8, 9, 10, 11 };
    mInner2 = { 12, 13, 14, 15 };
    mFillSeeds.push_back(Vector2<float>{ 128.0f, 32.0f });

    Triangulator triangulator(mPositions);
    std::vector<Triangulator::Polygon> inners(3);
    inners[i0] = mInner0;
    inners[i1] = mInner1;
    inners[i2] = mInner2;
    triangulator(mOuter, inners);
    mTriangles = triangulator.GetTriangles();
}
