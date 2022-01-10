#pragma once

#include <Mathematics/ArbitraryPrecision.h>
#include <Applications/Window2.h>
#include <Mathematics/ETManifoldMesh.h>
using namespace gte;

class UnionProjectionWindow2 : public Window2
{
public:
    UnionProjectionWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    using Rational = BSNumber<UIntegerAP32>;

    struct TriangleEx
    {
        TriangleEx()
            :
            exists(0),
            V{ 0, 0, 0 },
            A{ 0, 0, 0 },
            orientation(0)
        {
        }

        uint32_t exists;
        std::array<int32_t, 3> V;
        std::array<size_t, 3> A;
        int32_t orientation;
    };

    void LoadMesh();

    void ComputeUnionProjection(
        std::vector<Vector<3, float>> const& vertices,
        std::vector<std::array<int32_t, 3>> const& triangles,
        Vector<3, float> const& direction);

    void CreateSpiralMesh(std::vector<Vector3<float>>& outVertices,
        std::vector<std::array<int32_t, 3>>& outTriangles);

    inline void Get(Vector2<Rational> const& P, int32_t& x, int32_t& y)
    {
        //if (mPRange[0] >= mPRange[1])
        //{
        //    float xUnit = (float)(P[0] - mPMin[0]) / (float)mPRange[0];
        //    float yUnit = (float)(P[1] - mPMin[1]) / (float)mPRange[0];
        //    x = (int32_t)(mXSize * (0.01f + 0.99f * xUnit));
        //    y = (int32_t)(mXSize * (0.01f + 0.99f * yUnit));
        //}
        //else
        //{
        //    float xUnit = (float)(P[0] - mPMin[0]) / (float)mPRange[1];
        //    float yUnit = (float)(P[1] - mPMin[1]) / (float)mPRange[1];
        //    x = (int32_t)(mYSize * (0.01f + 0.99f * xUnit));
        //    y = (int32_t)(mYSize * (0.01f + 0.99f * yUnit));
        //}

        float xUnit = (float)(P[0] - mPMin[0]) / (float)mPRange[0];
        float yUnit = (float)(P[1] - mPMin[1]) / (float)mPRange[1];

        //float const xmin = 350.0f / mXSize, xmax = 416.0f / mXSize;
        //float const ymin = 410.0f / mYSize, ymax = 466.0f / mYSize;
        //xUnit = (xUnit - xmin) / (xmax - xmin);
        //yUnit = (yUnit - ymin) / (ymax - ymin);

        x = (int32_t)((mXSize - 1) * (0.01f + 0.99f * xUnit));
        y = (int32_t)((mYSize - 1) * (0.01f + 0.99f * yUnit));
    }

    std::vector<Vector2<Rational>> mProjections;
    Vector2<Rational> mPMin, mPMax, mPRange;
    std::vector<std::vector<EdgeKey<true>>> mPosPolygons;
    std::vector<std::vector<EdgeKey<true>>> mNegPolygons;
    std::set<EdgeKey<true>> mPosEdges;
    std::set<EdgeKey<true>> mNegEdges;
};
