// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Texture2Effect.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay2Mesh.h>
using namespace gte;

class Interpolation2DWindow3 : public Window3
{
public:
    Interpolation2DWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    bool SetEnvironment();
    void CreateCommonObjects();
    void OnPrecreateMesh();
    void OnPostcreateMesh();
    void CreateBilinearMesh();
    void CreateBicubicMesh(bool catmullRom);
    void CreateAkimaUniformMesh();
    void CreateThinPlateSplineMesh(float smooth);
    void CreateLinearNonuniform();
    void CreateQuadraticNonuniform(bool useGradients);

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::string mName;
    std::shared_ptr<Visual> mMesh;
    std::shared_ptr<Texture2> mTexture;
    std::shared_ptr<Texture2Effect> mEffect;
    std::shared_ptr<RasterizerState> mNoCullSolidState;
    std::shared_ptr<RasterizerState> mNoCullWireState;

    // For use by Bilinear, Bicubic, Akima, ThinPlateSpline.  The grid size
    // is SAMPLE_BOUND-by-SAMPLE_BOUND.
    static int32_t constexpr SAMPLE_BOUND = 8;
    static int32_t constexpr SAMPLE_BOUNDSQR = SAMPLE_BOUND * SAMPLE_BOUND;

    std::vector<float> mFSample;
    typedef BSNumber<UIntegerAP32> Numeric;
    typedef BSRational<UIntegerAP32> Rational;
    typedef Delaunay2Mesh<float, Numeric, Rational> TriangleMesh;
    Delaunay2<float, Numeric> mDelaunay;

    // For use by LinearNonuniform, QuadraticNonuniform.
    class SimpleMesh
    {
    public:
        SimpleMesh();

        int32_t GetNumVertices() const;
        int32_t GetNumTriangles() const;
        Vector2<float> const* GetVertices() const;
        int32_t const* GetIndices() const;
        bool GetVertices(int32_t t, std::array<Vector2<float>, 3>& vertices) const;
        bool GetIndices(int32_t t, std::array<int32_t, 3>& indices) const;
        bool GetAdjacencies(int32_t t, std::array<int32_t, 3>& adjacencies) const;
        bool GetBarycentrics(int32_t t, Vector2<float> const& P, std::array<float, 3>& bary) const;
        int32_t GetContainingTriangle(Vector2<float> const& P) const;

        inline int32_t GetInvalidIndex() const
        {
            return -1;
        }
    private:
        std::array<Vector2<float>, 6> mVertices;
        std::array<int32_t, 12> mIndices;
        std::array<int32_t, 12> mAdjacencies;
    };

    SimpleMesh mSimpleMesh;
    std::array<float, 6> mF, mDFDX, mDFDY;
};
