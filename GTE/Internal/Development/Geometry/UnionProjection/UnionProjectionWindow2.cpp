#include "UnionProjectionWindow2.h"
#include <Graphics/MeshFactory.h>
#include <queue>

UnionProjectionWindow2::UnionProjectionWindow2(Parameters& parameters)
    :
    Window2(parameters)
{
    LoadMesh();

    mDoFlip = true;
    OnDisplay();
}

void UnionProjectionWindow2::OnDisplay()
{
    uint32_t const white = 0xFFFFFFFF;
    uint32_t const blue = 0xFFF00000;
    uint32_t const red = 0xFF0000FF;

    ClearScreen(white);

    Vector2<Rational> P0{}, P1{};
    int32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;

    for (auto const& polygon : mNegPolygons)
    {
        for (auto const& edge : polygon)
        {
            P0 = mProjections[edge.V[0]];
            P1 = mProjections[edge.V[1]];
            Get(P0, x0, y0);
            Get(P1, x1, y1);
            DrawLine(x1, y1, x0, y0, red);
            //DrawLine(x0, y0, x1, y1, red);
        }
    }

    for (auto const& polygon : mPosPolygons)
    {
        for (auto const& edge : polygon)
        {
            P0 = mProjections[edge.V[0]];
            P1 = mProjections[edge.V[1]];
            Get(P0, x0, y0);
            Get(P1, x1, y1);
            DrawLine(x0, y0, x1, y1, blue);
        }
    }

    std::set<EdgeKey<true>> edgeUnion;
    for (auto const& polygon : mPosPolygons)
    {
        for (auto const& edge : polygon)
        {
            mPosEdges.insert(edge);
            edgeUnion.insert(edge);
        }
    }
    for (auto const& polygon : mNegPolygons)
    {
        for (auto const& edge : polygon)
        {
            mNegEdges.insert(edge);
            edgeUnion.insert(edge);
        }
    }

    std::set<EdgeKey<true>> edgeIntersection;
    for (auto const& edge : edgeUnion)
    {
        if (mPosEdges.find(edge) != mPosEdges.end() &&
            mNegEdges.find(edge) != mNegEdges.end())
        {
            edgeIntersection.insert(edge);
        }
    }

    mScreenTextureNeedsUpdate = true;
    Window2::OnDisplay();
}

void UnionProjectionWindow2::LoadMesh()
{
#if 1
    std::vector<Vector3<float>> vertices;
    std::vector<std::array<int32_t, 3>> triangles;
    CreateSpiralMesh(vertices, triangles);
#else
    std::ifstream input("TerrainVBuffer_P3T2_64x64.binary", std::ios::binary);
    std::vector<std::array<float, 5>> inputVertices(64 * 64);
    input.read((char*)inputVertices.data(), inputVertices.size() * sizeof(inputVertices[0]));
    input.close();

    input.open("TerrainIBuffer_UI32_64x64_6Tuple.binary", std::ios::binary);
    std::vector<std::array<int32_t, 6>> inputTriangles(63 * 63 * 2);
    input.read((char*)inputTriangles.data(), inputTriangles.size() * sizeof(inputTriangles[0]));
    input.close();

    std::vector<Vector<3, float>> vertices(inputVertices.size());
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto const& vertex = inputVertices[i];
        vertices[i] = { vertex[0], vertex[1], vertex[2] };
    }

    std::vector<std::array<int32_t, 3>> triangles(inputTriangles.size());
    for (size_t i = 0; i < triangles.size(); ++i)
    {
        auto const& triangle = inputTriangles[i];
        triangles[i] = { triangle[0], triangle[2], triangle[4] };
    }
#endif

    Vector<3, float> direction{ -0.5, 1.0, -0.5 };

    ComputeUnionProjection(vertices, triangles, direction);
}

void UnionProjectionWindow2::ComputeUnionProjection(
    std::vector<Vector<3, float>> const& vertices,
    std::vector<std::array<int32_t, 3>> const& triangles,
    Vector<3, float> const& direction)
{
    // Convert vertex components to rational numbers for error-free triangle
    // classification.
    std::vector<Vector3<Rational>> rvertices(vertices.size());
    for (size_t v = 0; v < vertices.size(); ++v)
    {
        for (int32_t i = 0; i < 3; ++i)
        {
            rvertices[v][i] = vertices[v][i];
        }
    }

    // Use the negative direction so that the triangles visible to the
    // viewer at infinity have positive orientation.
    Vector<3, Rational> negDirection = { -direction[0], -direction[1], -direction[2] };

    // Compute the triangle orientations. Store the positively oriented
    // triangles in posMesh and the negatively oriented triangles in negMesh.
    // The zero-oriented triangles are discarded because they do not have
    // any projection information that the other triangles have.
    ETManifoldMesh posMesh, negMesh;
    size_t numZero = 0;
    for (auto const& triangle : triangles)
    {
        auto edge10 = rvertices[triangle[1]] - rvertices[triangle[0]];
        auto edge20 = rvertices[triangle[2]] - rvertices[triangle[0]];
        auto orientation = DotCross(negDirection, edge10, edge20);
        int32_t sign = orientation.GetSign();

        if (sign > 0)
        {
            posMesh.Insert(triangle[0], triangle[1], triangle[2]);
        }
        else if (sign < 0)
        {
            negMesh.Insert(triangle[0], triangle[1], triangle[2]);
        }
        else
        {
            ++numZero;
        }
    }

    // Compute an orthonormal basis {U0,U1,D}. TODO: When using rational
    // arithmetic, required mutually perpendicular but not unit length
    // to avoid rounding errors. This leads to a nonuniform scaling, but
    // the union can be computed in this coordinate system without error.
    // The area can be computed and then inverse scaled to obtain the
    // area of the union without scaling.
    Rational const zero(0);
    Vector3<Rational> U0, U1;
    if (std::fabs(negDirection[0]) > std::fabs(negDirection[1]))
    {
        U0 = { -negDirection[2], zero, negDirection[0] };
    }
    else
    {
        U0 = { zero, negDirection[2], -negDirection[1] };
    }
    U1 = Cross(negDirection, U0);

    // Project the vertices onto the plane perpendicular to the direction.
    // With rational arithmetic, the center is not necessary. For
    // floating-point arithmetic, it helps with robustness.
    int32_t const numVertices = static_cast<int32_t>(vertices.size());
    Vector<3, float> vmin{}, vmax{};
    ComputeExtremes(numVertices, vertices.data(), vmin, vmax);
    mProjections.resize(vertices.size());
    Vector3<Rational> rvmin = { vmin[0], vmin[1], vmin[2] };
    Vector3<Rational> rvmax = { vmax[0], vmax[1], vmax[2] };
    Vector3<Rational> center = Rational(0.5) * (rvmin + rvmax);
    for (size_t v = 0; v < vertices.size(); ++v)
    {
        auto diff = rvertices[v] - center;
        mProjections[v][0] = Dot(U0, diff);
        mProjections[v][1] = Dot(U1, diff);
    }
    ComputeExtremes(numVertices, mProjections.data(), mPMin, mPMax);
    mPRange = mPMax - mPMin;

    // Get the connected components of the meshes. Each component is a
    // polygon (possibly with holes) that is used to compute the union
    // of polygons.
    std::vector<std::vector<ETManifoldMesh::Triangle*>> posComponents;
    std::vector<std::vector<ETManifoldMesh::Triangle*>> negComponents;
    posMesh.GetComponents(posComponents);
    negMesh.GetComponents(negComponents);
    // poscomp: 6971, 4, 12, 10, 2, 3, 5, 7, 2, 1, 3, 1, 4, 1, 2
    // negcomp: 36, 68, 29, 94, 69, 16, 280, 7, 15, 4, 59, 11,
    //   3, 31, 5, 3, 2, 14, 7, 23, 5, 20, 1, 4, 1, 3, 2, 2, 6,
    //   2, 3, 4, 20, 7, 10, 2, 2, 1, 2, 1, 5, 3, 1, 2, 3, 1, 2,
    //   1, 1, 2, 2, 1, 1, 1, 2, 3, 3, 1, 1

    // Get the boundaries of the connected components.
    mPosPolygons.resize(posComponents.size());
    for (size_t i = 0; i < posComponents.size(); ++i)
    {
        auto const& component = posComponents[i];
        auto& polygon = mPosPolygons[i];
        for (size_t j = 0; j < component.size(); ++j)
        {
            auto const* triangle = component[j];
            for (int32_t k0 = 2, k1 = 0; k1 < 3; k0 = k1++)
            {
                if (triangle->T[k0] == nullptr)
                {
                    polygon.push_back(EdgeKey<true>(triangle->V[k0], triangle->V[k1]));
                }
            }
        }
    }

    mNegPolygons.resize(negComponents.size());
    for (size_t i = 0; i < negComponents.size(); ++i)
    {
        auto const& component = negComponents[i];
        auto& polygon = mNegPolygons[i];
        for (size_t j = 0; j < component.size(); ++j)
        {
            auto const* triangle = component[j];
            for (int32_t k0 = 2, k1 = 0; k1 < 3; k0 = k1++)
            {
                if (triangle->T[k0] == nullptr)
                {
                    polygon.push_back(EdgeKey<true>(triangle->V[k1], triangle->V[k0]));
                }
            }
        }
    }

    int stophere;
    stophere = 0;
}

void UnionProjectionWindow2::CreateSpiralMesh(std::vector<Vector3<float>>& outVertices,
    std::vector<std::array<int32_t, 3>>& outTriangles)
{
    outVertices.resize(64 * 65);

    MeshFactory mf;
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    mf.SetVertexFormat(vformat);
    auto spiral = mf.CreateCylinderOpen(64, 64, 1.0, 1.0);
    float const radius = 1.0f;
    float const pi = static_cast<float>(GTE_C_PI);
    for (size_t row = 0, i = 0; row < 64; ++row)
    {
        float t = row / 63.0f;
        float angle = 4.0f * pi * t;
        float cs = std::cos(angle);
        float sn = std::sin(angle);
        Vector<3, float> center{ 8.0f * cs, 8.0f * sn, -2.0f + 4.0f * t };
        Vector<3, float> U{ cs, sn, 0.0f };
        Vector<3, float> V{ -sn, cs, 1.0f / (8.0f * pi) };
        for (size_t col = 0; col <= 64; ++col, ++i)
        {
            float phi = 2.0f * pi * col / 64.0f;
            float rcsphi = radius * std::cos(phi);
            float rsnphi = radius * std::sin(phi);
            outVertices[i] = center + rcsphi * U + rsnphi * V;
        }
    }

    auto const& ibuffer = spiral->GetIndexBuffer();
    auto const* triangles = ibuffer->Get<std::array<int32_t, 3>>();
    outTriangles.resize(ibuffer->GetNumPrimitives());
    for (size_t t = 0; t < outTriangles.size(); ++t)
    {
        outTriangles[t] = triangles[t];
    }
}