// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.09.30

#include "PolygonBooleanOperationsWindow2.h"

PolygonBooleanOperationsWindow2::PolygonBooleanOperationsWindow2(Parameters& parameters)
    :
    Window2(parameters),
#if defined(USE_RATIONAL_ARITHMETIC)
    mEpsilon(0),
#else
    mEpsilon(0.00001),
#endif
    mIntersection(mEpsilon),
    mUnion(mEpsilon),
    mDiff01(mEpsilon),
    mDiff10(mEpsilon),
    mXor(mEpsilon),
    mActive(nullptr),
    mChoice(0),
    mSize(static_cast<Numeric>(mXSize))
{
    mPoly0 = ConstructInvertedEll();
    mPoly1 = ConstructPentagon();
    DoBoolean();
    OnDisplay();
}

void PolygonBooleanOperationsWindow2::OnDisplay()
{
    uint32_t white = 0xFFFFFFFF;
    uint32_t red = 0xFF0000FF;
    uint32_t green = 0xFF00FF00;
    uint32_t blue = 0xFFFF0000;

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

bool PolygonBooleanOperationsWindow2::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

std::unique_ptr<BSPPolygon2<Numeric>> PolygonBooleanOperationsWindow2::ConstructInvertedEll()
{
    Numeric d1d8 = static_cast<Numeric>(0.125) * mSize;
    Numeric d2d8 = static_cast<Numeric>(0.250) * mSize;
    Numeric d3d8 = static_cast<Numeric>(0.375) * mSize;
    Numeric d5d8 = static_cast<Numeric>(0.625) * mSize;
    Numeric d6d8 = static_cast<Numeric>(0.750) * mSize;
    Numeric d7d8 = static_cast<Numeric>(0.875) * mSize;

    int32_t constexpr numVertices = 10;
    std::array<Vector2<Numeric>, numVertices> vertices =
    {
        Vector2<Numeric>{ d1d8, d1d8 },
        Vector2<Numeric>{ d3d8, d1d8 },
        Vector2<Numeric>{ d3d8, d3d8 },
        Vector2<Numeric>{ d2d8, d3d8 },
        Vector2<Numeric>{ d2d8, d6d8 },
        Vector2<Numeric>{ d5d8, d6d8 },
        Vector2<Numeric>{ d5d8, d5d8 },
        Vector2<Numeric>{ d7d8, d5d8 },
        Vector2<Numeric>{ d7d8, d7d8 },
        Vector2<Numeric>{ d1d8, d7d8 }
    };

    auto poly = std::make_unique<BSPPolygon2<Numeric>>(mEpsilon);
    for (int32_t i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<Numeric>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<Numeric>> PolygonBooleanOperationsWindow2::ConstructPentagon()
{
    int32_t constexpr numVertices = 5;

    Numeric primitiveAngle = static_cast<Numeric>(GTE_C_TWO_PI) / static_cast<Numeric>(numVertices);
    Numeric radius = static_cast<Numeric>(35) / static_cast<Numeric>(100) * mSize;
    Numeric cx = static_cast<Numeric>(0.5) * mSize;
    Numeric cy = static_cast<Numeric>(0.5) * mSize;

    std::array<Vector2<Numeric>, numVertices> vertices{};
    for (int32_t i = 0; i < numVertices; ++i)
    {
        Numeric angle = static_cast<Numeric>(i) * primitiveAngle;
        vertices[i][0] = cx + radius * std::cos(angle);
        vertices[i][1] = cy + radius * std::sin(angle);
    }

    auto poly = std::make_unique<BSPPolygon2<Numeric>>(mEpsilon);
    for (int32_t i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<Numeric>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<Numeric>> PolygonBooleanOperationsWindow2::ConstructSquare()
{
    Numeric d2d8 = static_cast<Numeric>(0.250) * mSize;
    Numeric d6d8 = static_cast<Numeric>(0.750) * mSize;

    int32_t constexpr numVertices = 4;
    std::array<Vector2<Numeric>, numVertices> vertices =
    {
        Vector2<Numeric>{ d2d8, d2d8 },
        Vector2<Numeric>{ d6d8, d2d8 },
        Vector2<Numeric>{ d6d8, d6d8 },
        Vector2<Numeric>{ d2d8, d6d8 }
    };

    auto poly = std::make_unique<BSPPolygon2<Numeric>>(mEpsilon);
    for (int32_t i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<Numeric>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<Numeric>> PolygonBooleanOperationsWindow2::ConstructSShape()
{
    Numeric d10d32 = static_cast<Numeric>(10) * mSize / static_cast<Numeric>(32);
    Numeric d12d32 = static_cast<Numeric>(12) * mSize / static_cast<Numeric>(32);
    Numeric d13d32 = static_cast<Numeric>(13) * mSize / static_cast<Numeric>(32);
    Numeric d16d32 = static_cast<Numeric>(16) * mSize / static_cast<Numeric>(32);
    Numeric d19d32 = static_cast<Numeric>(19) * mSize / static_cast<Numeric>(32);
    Numeric d20d32 = static_cast<Numeric>(20) * mSize / static_cast<Numeric>(32);
    Numeric d22d32 = static_cast<Numeric>(22) * mSize / static_cast<Numeric>(32);
    Numeric d24d32 = static_cast<Numeric>(24) * mSize / static_cast<Numeric>(32);
    Numeric d26d32 = static_cast<Numeric>(26) * mSize / static_cast<Numeric>(32);
    Numeric d28d32 = static_cast<Numeric>(28) * mSize / static_cast<Numeric>(32);

    int32_t constexpr numVertices = 12;
    std::array<Vector2<Numeric>, numVertices> vertices =
    {
        Vector2<Numeric>{ d24d32, d10d32 },
        Vector2<Numeric>{ d28d32, d10d32 },
        Vector2<Numeric>{ d28d32, d16d32 },
        Vector2<Numeric>{ d22d32, d16d32 },
        Vector2<Numeric>{ d22d32, d19d32 },
        Vector2<Numeric>{ d24d32, d19d32 },
        Vector2<Numeric>{ d24d32, d22d32 },
        Vector2<Numeric>{ d20d32, d22d32 },
        Vector2<Numeric>{ d20d32, d13d32 },
        Vector2<Numeric>{ d26d32, d13d32 },
        Vector2<Numeric>{ d26d32, d12d32 },
        Vector2<Numeric>{ d24d32, d12d32 }
    };

    auto poly = std::make_unique<BSPPolygon2<Numeric>>(mEpsilon);
    for (int32_t i0 = numVertices - 1, i1 = 0; i1 < numVertices; i0 = i1++)
    {
        poly->InsertVertex(vertices[i1]);
        poly->InsertEdge(BSPPolygon2<Numeric>::Edge(i0, i1));
    }
    poly->Finalize();
    return poly;
}

std::unique_ptr<BSPPolygon2<Numeric>> PolygonBooleanOperationsWindow2::ConstructPolyWithHoles()
{
    Numeric d2d16 = static_cast<Numeric>(2) * mSize / static_cast<Numeric>(16);
    Numeric d3d16 = static_cast<Numeric>(3) * mSize / static_cast<Numeric>(16);
    Numeric d4d16 = static_cast<Numeric>(4) * mSize / static_cast<Numeric>(16);
    Numeric d6d16 = static_cast<Numeric>(6) * mSize / static_cast<Numeric>(16);
    Numeric d14d16 = static_cast<Numeric>(14) * mSize / static_cast<Numeric>(16);

    int32_t constexpr numVertices = 6;
    std::array<Vector2<Numeric>, numVertices> vertices =
    {
        // outer boundary
        Vector2<Numeric>{ d2d16, d2d16 },
        Vector2<Numeric>{ d14d16, d2d16 },
        Vector2<Numeric>{ d2d16, d14d16 },

        // inner boundary
        Vector2<Numeric>{ d4d16, d3d16 },
        Vector2<Numeric>{ d6d16, d6d16 },
        Vector2<Numeric>{ d6d16, d3d16 }
    };

    auto poly = std::make_unique<BSPPolygon2<Numeric>>(mEpsilon);
    for (int32_t i = 0; i < numVertices; ++i)
    {
        poly->InsertVertex(vertices[i]);
    }

    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(0, 1));
    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(1, 2));
    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(2, 0));
    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(3, 4));
    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(4, 5));
    poly->InsertEdge(BSPPolygon2<Numeric>::Edge(5, 3));

    poly->Finalize();
    return poly;
}

void PolygonBooleanOperationsWindow2::DrawPolySolid(BSPPolygon2<Numeric>& polygon, uint32_t color)
{
    // Draw the edges.
    for (int32_t i = 0; i < polygon.GetNumEdges(); ++i)
    {
        BSPPolygon2<Numeric>::Edge edge = polygon.GetEdge(i);
        Vector2<Numeric> v0 = polygon.GetVertex(edge.V[0]);
        Vector2<Numeric> v1 = polygon.GetVertex(edge.V[1]);

        int32_t x0 = static_cast<int32_t>(static_cast<double>(v0[0] + static_cast<Numeric>(0.5)));
        int32_t y0 = mXSize - 1 - static_cast<int32_t>(static_cast<double>(v0[1] + static_cast<Numeric>(0.5)));
        int32_t x1 = static_cast<int32_t>(static_cast<double>(v1[0] + static_cast<Numeric>(0.5)));
        int32_t y1 = mYSize - 1 - static_cast<int32_t>(static_cast<double>(v1[1] + static_cast<Numeric>(0.5)));

        DrawLine(x0, y0, x1, y1, color);
    }

    // Draw the vertices.
    uint32_t black = 0xFF000000;
    for (int32_t i = 0; i < polygon.GetNumVertices(); ++i)
    {
        Vector2<Numeric> v = polygon.GetVertex(i);
        int32_t x = static_cast<int32_t>(static_cast<double>(v[0] + static_cast<Numeric>(0.5)));
        int32_t y = mYSize - 1 - static_cast<int32_t>(static_cast<double>(v[1] + static_cast<Numeric>(0.5)));
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
