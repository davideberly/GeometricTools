// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "PolygonBooleanOperationsWindow2.h"

PolygonBooleanOperationsWindow2::PolygonBooleanOperationsWindow2(Parameters& parameters)
    :
    Window2(parameters),
    mEpsilon(0.00001),
    mIntersection(mEpsilon),
    mUnion(mEpsilon),
    mDiff01(mEpsilon),
    mDiff10(mEpsilon),
    mXor(mEpsilon),
    mActive(nullptr),
    mChoice(0),
    mSize(static_cast<double>(mXSize))
{
    mPoly0 = ConstructInvertedEll();
    mPoly1 = ConstructPentagon();
    DoBoolean();
    OnDisplay();
}

void PolygonBooleanOperationsWindow2::OnDisplay()
{
    unsigned int white = 0xFFFFFFFF;
    unsigned int red = 0xFF0000FF;
    unsigned int green = 0xFF00FF00;
    unsigned int blue = 0xFFFF0000;

    ClearScreen(white);

    DrawPolySolid(*mPoly0, red);
    DrawPolySolid(*mPoly1, green);
    if (mActive)
    {
        DrawPolySolid(*mActive, blue);
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

bool PolygonBooleanOperationsWindow2::OnCharPress(unsigned char key, int x, int y)
{
    if (Window2::OnCharPress(key, x, y))
    {
        return true;
    }

    switch (key)
    {
    case 'n':
    case 'N':
        mPoly0 = nullptr;
        mPoly1 = nullptr;
        mActive = nullptr;

        mChoice = (mChoice + 1) % 3;
        switch (mChoice)
        {
        case 0:
            mPoly0 = ConstructInvertedEll();
            mPoly1 = ConstructPentagon();
            break;
        case 1:
            mPoly0 = ConstructSquare();
            mPoly1 = ConstructSShape();
            break;
        case 2:
            mPoly0 = ConstructPolyWithHoles();
            mPoly1 = ConstructPentagon();
            break;
        }
        DoBoolean();
        break;

    case 'p':
    case 'P':
        mActive = nullptr;
        break;
    case 'u':
    case 'U':
        mActive = &mUnion;
        break;
    case 'i':
    case 'I':
        mActive = &mIntersection;
        break;
    case 'd':
    case 'D':
        mActive = &mDiff01;
        break;
    case 'e':
    case 'E':
        mActive = &mDiff10;
        break;
    case 'x':
    case 'X':
        mActive = &mXor;
        break;
    }

    OnDisplay();
    return true;
}

std::unique_ptr<BSPPolygon2<double>> PolygonBooleanOperationsWindow2::ConstructInvertedEll()
{
    double d1d8 = 0.125 * mSize;
    double d2d8 = 0.250 * mSize;
    double d3d8 = 0.375 * mSize;
    double d5d8 = 0.625 * mSize;
    double d6d8 = 0.750 * mSize;
    double d7d8 = 0.875 * mSize;

    int const numVertices = 10;
    Vector2<double> vertices[numVertices] =
    {
        Vector2<double>{ d1d8, d1d8 },
        Vector2<double>{ d3d8, d1d8 },
        Vector2<double>{ d3d8, d3d8 },
        Vector2<double>{ d2d8, d3d8 },
        Vector2<double>{ d2d8, d6d8 },
        Vector2<double>{ d5d8, d6d8 },
        Vector2<double>{ d5d8, d5d8 },
        Vector2<double>{ d7d8, d5d8 },
        Vector2<double>{ d7d8, d7d8 },
        Vector2<double>{ d1d8, d7d8 }
    };

    auto poly = std::make_unique<BSPPolygon2<double>>(mEpsilon);
    for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<double>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<double>> PolygonBooleanOperationsWindow2::ConstructPentagon()
{
    int const numVertices = 5;

    double primitiveAngle = GTE_C_TWO_PI / numVertices;
    double radius = 0.35 * mSize;
    double cx = 0.5 * mSize;
    double cy = 0.5 * mSize;

    Vector2<double> vertices[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        double angle = i * primitiveAngle;
        vertices[i][0] = cx + radius * std::cos(angle);
        vertices[i][1] = cy + radius * std::sin(angle);
    }

    auto poly = std::make_unique<BSPPolygon2<double>>(mEpsilon);
    for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<double>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<double>> PolygonBooleanOperationsWindow2::ConstructSquare()
{
    double d2d8 = 0.250 * mSize;
    double d6d8 = 0.750 * mSize;

    int const numVertices = 4;
    Vector2<double> vertices[numVertices] =
    {
        Vector2<double>{ d2d8, d2d8 },
        Vector2<double>{ d6d8, d2d8 },
        Vector2<double>{ d6d8, d6d8 },
        Vector2<double>{ d2d8, d6d8 }
    };

    auto poly = std::make_unique<BSPPolygon2<double>>(mEpsilon);
    for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<double>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<double>> PolygonBooleanOperationsWindow2::ConstructSShape()
{
    double d10d32 = 10.0 * mSize / 32.0;
    double d12d32 = 12.0 * mSize / 32.0;
    double d13d32 = 13.0 * mSize / 32.0;
    double d16d32 = 16.0 * mSize / 32.0;
    double d19d32 = 19.0 * mSize / 32.0;
    double d20d32 = 20.0 * mSize / 32.0;
    double d22d32 = 22.0 * mSize / 32.0;
    double d24d32 = 24.0 * mSize / 32.0;
    double d26d32 = 26.0 * mSize / 32.0;
    double d28d32 = 28.0 * mSize / 32.0;

    int const numVertices = 12;
    Vector2<double> vertices[numVertices] =
    {
        Vector2<double>{ d24d32, d10d32 },
        Vector2<double>{ d28d32, d10d32 },
        Vector2<double>{ d28d32, d16d32 },
        Vector2<double>{ d22d32, d16d32 },
        Vector2<double>{ d22d32, d19d32 },
        Vector2<double>{ d24d32, d19d32 },
        Vector2<double>{ d24d32, d22d32 },
        Vector2<double>{ d20d32, d22d32 },
        Vector2<double>{ d20d32, d13d32 },
        Vector2<double>{ d26d32, d13d32 },
        Vector2<double>{ d26d32, d12d32 },
        Vector2<double>{ d24d32, d12d32 }
    };

    auto poly = std::make_unique<BSPPolygon2<double>>(mEpsilon);
    for (int i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<double>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<double>> PolygonBooleanOperationsWindow2::ConstructPolyWithHoles()
{
    double d2d16 = 2.0 * mSize / 16.0;
    double d3d16 = 3.0 * mSize / 16.0;
    double d4d16 = 4.0 * mSize / 16.0;
    double d6d16 = 6.0 * mSize / 16.0;
    double d14d16 = 14.0 * mSize / 16.0;

    int const numVertices = 6;
    Vector2<double> vertices[numVertices] =
    {
        // outer boundary
        Vector2<double>{ d2d16, d2d16 },
        Vector2<double>{ d14d16, d2d16 },
        Vector2<double>{ d2d16, d14d16 },

        // inner boundary
        Vector2<double>{ d4d16, d3d16 },
        Vector2<double>{ d6d16, d6d16 },
        Vector2<double>{ d6d16, d3d16 }
    };

    auto poly = std::make_unique<BSPPolygon2<double>>(mEpsilon);
    for (int i = 0; i < numVertices; ++i)
    {
        poly->InsertVertex(vertices[i]);
    }

    poly->InsertEdge(BSPPolygon2<double>::Edge(0, 1));
    poly->InsertEdge(BSPPolygon2<double>::Edge(1, 2));
    poly->InsertEdge(BSPPolygon2<double>::Edge(2, 0));
    poly->InsertEdge(BSPPolygon2<double>::Edge(3, 4));
    poly->InsertEdge(BSPPolygon2<double>::Edge(4, 5));
    poly->InsertEdge(BSPPolygon2<double>::Edge(5, 3));

    poly->Finalize();
    return poly;
}

void PolygonBooleanOperationsWindow2::DrawPolySolid(BSPPolygon2<double>& polygon, unsigned int color)
{
    // Draw the edges.
    for (int i = 0; i < polygon.GetNumEdges(); ++i)
    {
        BSPPolygon2<double>::Edge edge = polygon.GetEdge(i);
        Vector2<double> v0 = polygon.GetVertex(edge.V[0]);
        Vector2<double> v1 = polygon.GetVertex(edge.V[1]);

        int x0 = static_cast<int>(v0[0] + 0.5f);
        int y0 = mXSize - 1 - static_cast<int>(v0[1] + 0.5f);
        int x1 = static_cast<int>(v1[0] + 0.5f);
        int y1 = mYSize - 1 - static_cast<int>(v1[1] + 0.5f);

        DrawLine(x0, y0, x1, y1, color);
    }

    // Draw the vertices.
    unsigned int black = 0xFF000000;
    for (int i = 0; i < polygon.GetNumVertices(); ++i)
    {
        Vector2<double> v = polygon.GetVertex(i);
        int x = static_cast<int>(v[0] + 0.5f);
        int y = mYSize - 1 - static_cast<int>(v[1] + 0.5f);
        DrawThickPixel(x, y, 1, black);
    }
}

void PolygonBooleanOperationsWindow2::DoBoolean()
{
    auto const& P = *mPoly0;
    auto const& Q = *mPoly1;

    mIntersection = P & Q;
    mUnion = P | Q;
    mDiff01 = P - Q;
    mDiff10 = Q - P;
    mXor = P ^ Q;
}
