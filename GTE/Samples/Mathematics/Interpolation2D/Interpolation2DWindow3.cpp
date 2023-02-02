// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.06.08

#include "Interpolation2DWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Mathematics/IntpAkimaUniform2.h>
#include <Mathematics/IntpBicubic2.h>
#include <Mathematics/IntpBilinear2.h>
#include <Mathematics/IntpLinearNonuniform2.h>
#include <Mathematics/IntpQuadraticNonuniform2.h>
#include <Mathematics/IntpThinPlateSpline2.h>
#include <random>

Interpolation2DWindow3::Interpolation2DWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    CreateCommonObjects();
    CreateBilinearMesh();
    OnDisplay();
}

void Interpolation2DWindow3::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mMesh);
    mEngine->Draw(8, 24, { 0.0f, 0.0f, 0.0f, 1.0f }, mName);
    mEngine->DisplayColorBuffer(0);
}

bool Interpolation2DWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case '0':
        if (mName != "Bilinear2")
        {
            CreateBilinearMesh();
        }
        return true;
    case '1':
        if (mName != "Bicubic2_CM")
        {
            CreateBicubicMesh(true);
        }
        return true;
    case '2':
        if (mName != "Bicubic2_BS")
        {
            CreateBicubicMesh(false);
        }
        return true;
    case '3':
        if (mName != "AkimaUniform2_BS")
        {
            CreateAkimaUniformMesh();
        }
        return true;
    case '4':
        if (mName != "ThinPlateSpline2_0")
        {
            CreateThinPlateSplineMesh(0.0f);
        }
        return true;
    case '5':
        if (mName != "ThinPlateSpline2_1")
        {
            CreateThinPlateSplineMesh(1.0f);
        }
        return true;
    case '6':
        if (mName != "ThinPlateSpline2_10")
        {
            CreateThinPlateSplineMesh(10.0f);
        }
        return true;
    case '7':
        if (mName != "LinearNonuniform")
        {
            CreateLinearNonuniform();
        }
        return true;
    case '8':
        if (mName != "QuadraticNonuniform_NOGRAD")
        {
            CreateQuadraticNonuniform(false);
        }
        return true;
    case '9':
        if (mName != "QuadraticNonuniform_GRAD")
        {
            CreateQuadraticNonuniform(true);
        }
        return true;
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullSolidState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullSolidState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool Interpolation2DWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Checkerboard.png") == "")
    {
        LogError("Cannot find file Checkerboard.png.");
        return false;
    }

    return true;
}

void Interpolation2DWindow3::CreateCommonObjects()
{
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 0.0f, 0.0f, 2.5f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });

    std::string path = mEnvironment.GetPath("Checkerboard.png");
    mTexture = WICFileIO::Load(path, true);
    mTexture->AutogenerateMipmaps();
    mEffect = std::make_shared<Texture2Effect>(mProgramFactory, mTexture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);

    mNoCullSolidState = std::make_shared<RasterizerState>();
    mNoCullSolidState->cull = RasterizerState::Cull::NONE;
    mNoCullSolidState->fill = RasterizerState::Fill::SOLID;
    mEngine->SetRasterizerState(mNoCullSolidState);

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;

    float const maxRnd = 0.125f;
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, maxRnd);
    mFSample.resize(SAMPLE_BOUNDSQR);
    for (int32_t y = 0; y < SAMPLE_BOUND; ++y)
    {
        for (int32_t x = 0; x < SAMPLE_BOUND; ++x)
        {
            mFSample[x + static_cast<size_t>(SAMPLE_BOUND) * y] = rnd(mte);
        }
    }

    mF[0] = 0.0f;   mDFDX[0] = -0.125f;     mDFDY[0] = -0.125f;
    mF[1] = 1.0f;   mDFDX[1] = 0.0f;        mDFDY[1] = -0.25f;
    mF[2] = 0.5f;   mDFDX[2] = 0.25f;       mDFDY[2] = 0.0f;
    mF[3] = 1.0f;   mDFDX[3] = -0.0625f;    mDFDY[3] = 0.0625f;
    mF[4] = 1.0f;   mDFDX[4] = 0.0625f;     mDFDY[4] = -0.0625f;
    mF[5] = 0.25f;  mDFDX[5] = 0.0f;        mDFDY[5] = -0.25f;

    std::fill(mDFDX.begin(), mDFDX.end(), 0.0f);
    std::fill(mDFDY.begin(), mDFDY.end(), 0.0f);
}

void Interpolation2DWindow3::OnPrecreateMesh()
{
    if (mMesh)
    {
        mTrackBall.Detach(mMesh);
        mPVWMatrices.Unsubscribe(mMesh->worldTransform);
    }
}

void Interpolation2DWindow3::OnPostcreateMesh()
{
    mMesh->SetEffect(mEffect);
    mPVWMatrices.Subscribe(mMesh->worldTransform, mEffect->GetPVWMatrixConstant());
    mTrackBall.Attach(mMesh);
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void Interpolation2DWindow3::CreateBilinearMesh()
{
    OnPrecreateMesh();

    mName = "Bilinear2";

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpBilinear2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f, spacing,
        -1.0f, spacing, &mFSample[0]);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int32_t y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int32_t x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow3::CreateBicubicMesh(bool catmullRom)
{
    OnPrecreateMesh();

    if (catmullRom)
    {
        mName = "Bicubic2_CR";
    }
    else
    {
        mName = "Bicubic2_BS";
    }

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpBicubic2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f, spacing,
        -1.0f, spacing, &mFSample[0], catmullRom);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int32_t y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int32_t x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow3::CreateAkimaUniformMesh()
{
    OnPrecreateMesh();

    mName = "AkimaUniform2";

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    IntpAkimaUniform2<float> interp(SAMPLE_BOUND, SAMPLE_BOUND, -1.0f,
        spacing, -1.0f, spacing, &mFSample[0]);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int32_t y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int32_t x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow3::CreateThinPlateSplineMesh(float smooth)
{
    OnPrecreateMesh();

    if (smooth == 0.0f)
    {
        mName = "ThinPlateSpline2_0";
    }
    else if (smooth == 1.0f)
    {
        mName = "ThinPlateSpline2_1";
    }
    else
    {
        mName = "ThinPlateSpline2_10";
    }

    float const spacing = 2.0f / static_cast<float>(SAMPLE_BOUND - 1);
    std::vector<float> xDomain(SAMPLE_BOUNDSQR);
    std::vector<float> yDomain(SAMPLE_BOUNDSQR);
    for (int32_t y = 0, i = 0; y < SAMPLE_BOUND; ++y)
    {
        float fy = -1.0f + spacing * y;
        for (int32_t x = 0; x < SAMPLE_BOUND; ++x, ++i)
        {
            float fx = -1.0f + spacing * x;
            xDomain[i] = fx;
            yDomain[i] = fy;
        }
    }

    IntpThinPlateSpline2<float> interp(SAMPLE_BOUNDSQR, &xDomain[0],
        &yDomain[0], &mFSample[0], smooth, false);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 64;
    mMesh = mf.CreateRectangle(numSamples, numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    for (int32_t y = 0, i = 0; y < numSamples; ++y)
    {
        float fy = vertices[i].position[1];
        for (int32_t x = 0; x < numSamples; ++x, ++i)
        {
            float fx = vertices[i].position[0];
            vertices[i].position[2] = interp(fx, fy);
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow3::CreateLinearNonuniform()
{
    OnPrecreateMesh();

    mName = "LinearNonuniform";

    IntpLinearNonuniform2<float, SimpleMesh> interp(mSimpleMesh, &mF[0]);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 64;
    mMesh = mf.CreateTriangle(numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    Vector2<float> P{};
    float F{};
    for (int32_t y = 0, i = 0; y < numSamples; ++y)
    {
        for (int32_t x = 0; x + y < numSamples; ++x, ++i)
        {
            // For the sake of the demonstration, ensure P is inside the
            // domain of the interpolator for all (x,y).
            P[0] = std::min(std::max(vertices[i].position[0], 0.0f), 1.0f);
            P[1] = std::min(std::max(vertices[i].position[1], 0.0f), 1.0f);
            float sum = P[0] + P[1];
            if (sum > 1.0f)
            {
                P[0] /= sum;
                P[1] /= sum;
            }

            if (interp(P, F))
            {
                vertices[i].position[2] = F;
            }
            else
            {
                // We should not get here with the adjustment above.
                vertices[i].position[2] = 0.0f;
            }
        }
    }

    OnPostcreateMesh();
}

void Interpolation2DWindow3::CreateQuadraticNonuniform(bool useGradients)
{
    OnPrecreateMesh();

    std::unique_ptr<IntpQuadraticNonuniform2<float, SimpleMesh>> interp;
    if (useGradients)
    {
        mName = "QuadraticNonuniform_GRAD";
        interp = std::make_unique<IntpQuadraticNonuniform2<float, SimpleMesh>>(
            mSimpleMesh, &mF[0], &mDFDX[0], &mDFDY[0]);
    }
    else
    {
        mName = "QuadraticNonuniform_NOGRAD";
        float spatialDelta = 0.125f;
        interp = std::make_unique<IntpQuadraticNonuniform2<float, SimpleMesh>>(
            mSimpleMesh, &mF[0], spatialDelta);
    }

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    int32_t const numSamples = 128;
    mMesh = mf.CreateTriangle(numSamples, 1.0f, 1.0f);
    auto vertices = mMesh->GetVertexBuffer()->Get<Vertex>();
    Vector2<float> P{};
    float F{}, FX{}, FY{};
    int32_t i = 0;
    for (int32_t y = 0; y < numSamples; ++y)
    {
        for (int32_t x = 0; x + y < numSamples; ++x, ++i)
        {
            // For the sake of the demonstration, ensure P is inside the
            // domain of the interpolator for all (x,y).
            P[0] = std::min(std::max(vertices[i].position[0], 0.0f), 1.0f);
            P[1] = std::min(std::max(vertices[i].position[1], 0.0f), 1.0f);
            float sum = P[0] + P[1];
            if (sum > 1.0f)
            {
                P[0] /= sum;
                P[1] /= sum;
            }

            if ((*interp)(P, F, FX, FY))
            {
                vertices[i].position[2] = F;
            }
            else
            {
                // We should not get here with the adjustment above.
                vertices[i].position[2] = 0.0f;
            }
        }
    }

    OnPostcreateMesh();
}

Interpolation2DWindow3::SimpleMesh::SimpleMesh()
{
    mVertices[0] = { 0.0f, 0.0f };
    mVertices[1] = { 0.5f, 0.0f };
    mVertices[2] = { 1.0f, 0.0f };
    mVertices[3] = { 0.0f, 0.5f };
    mVertices[4] = { 0.5f, 0.5f };
    mVertices[5] = { 0.0f, 1.0f };

    mIndices[0] = 0;  mIndices[1] = 1;  mIndices[2] = 3;
    mIndices[3] = 1;  mIndices[4] = 2;  mIndices[5] = 4;
    mIndices[6] = 3;  mIndices[7] = 4;  mIndices[8] = 5;
    mIndices[9] = 1;  mIndices[10] = 4;  mIndices[11] = 3;

    mAdjacencies[0] = -1;  mAdjacencies[1] = 3;  mAdjacencies[2] = -1;
    mAdjacencies[3] = -1;  mAdjacencies[4] = -1;  mAdjacencies[5] = 3;
    mAdjacencies[6] = 3;  mAdjacencies[7] = -1;  mAdjacencies[8] = -1;
    mAdjacencies[9] = 0;  mAdjacencies[10] = 1;  mAdjacencies[11] = 2;
}

int32_t Interpolation2DWindow3::SimpleMesh::GetNumVertices() const
{
    return 6;
}

int32_t Interpolation2DWindow3::SimpleMesh::GetNumTriangles() const
{
    return 4;
}

Vector2<float> const* Interpolation2DWindow3::SimpleMesh::GetVertices() const
{
    return &mVertices[0];
}

int32_t const* Interpolation2DWindow3::SimpleMesh::GetIndices() const
{
    return &mIndices[0];
}

bool Interpolation2DWindow3::SimpleMesh::GetVertices(int32_t t,
    std::array<Vector2<float>, 3>& vertices) const
{
    std::array<int32_t, 3> indices;
    if (GetIndices(t, indices))
    {
        vertices[0] = mVertices[indices[0]];
        vertices[1] = mVertices[indices[1]];
        vertices[2] = mVertices[indices[2]];
        return true;
    }
    return false;
}

bool Interpolation2DWindow3::SimpleMesh::GetIndices(int32_t t,
    std::array<int32_t, 3>& indices) const
{
    if (0 <= t && t < 4)
    {
        size_t threeT = 3 * static_cast<size_t>(t);
        indices[0] = mIndices[threeT + 0];
        indices[1] = mIndices[threeT + 1];
        indices[2] = mIndices[threeT + 2];
        return true;
    }
    return false;
}

bool Interpolation2DWindow3::SimpleMesh::GetAdjacencies(int32_t t,
    std::array<int32_t, 3>& adjacencies) const
{
    if (0 <= t && t < 4)
    {
        size_t threeT = 3 * static_cast<size_t>(t);
        adjacencies[0] = mAdjacencies[threeT + 0];
        adjacencies[1] = mAdjacencies[threeT + 1];
        adjacencies[2] = mAdjacencies[threeT + 2];
        return true;
    }
    return false;
}

bool Interpolation2DWindow3::SimpleMesh::GetBarycentrics(int32_t t,
    Vector2<float> const& P, std::array<float, 3>& bary) const
{
    std::array<Vector2<float>, 3> V{};
    if (GetVertices(t, V))
    {
        return ComputeBarycentrics(P, V[0], V[1], V[2], bary, 0.0f);
    }
    return false;
}

int32_t Interpolation2DWindow3::SimpleMesh::GetContainingTriangle(
    Vector2<float> const& P) const
{
    if (P[0] < 0.0f || P[1] < 0.0f || P[0] + P[1] > 1.0f)
    {
        return -1;
    }

    if (P[0] + P[1] <= 0.5f)
    {
        return 0;
    }

    if (P[0] >= 0.5f)
    {
        return 1;
    }

    if (P[1] >= 0.5f)
    {
        return 2;
    }

    return 3;
}
