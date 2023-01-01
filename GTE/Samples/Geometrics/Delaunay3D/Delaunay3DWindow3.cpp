// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "Delaunay3DWindow3.h"
#include <Graphics/MeshFactory.h>

Delaunay3DWindow3::Delaunay3DWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mLightGray{ 0.75f, 0.75f, 0.75f, 1.0f }
{
    if (!SetEnvironment() || !CreateScene())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -4.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mPVWMatrices.Update();
}

void Delaunay3DWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    mEngine->SetRasterizerState(mNoCullWireState);
    for (auto const& tetra : mWireTetra)
    {
        if (tetra)
        {
            mEngine->Draw(tetra);
        }
    }

    mEngine->SetBlendState(mBlendState);
    mEngine->SetRasterizerState(mNoCullState);
    for (auto const& tetra : mSolidTetra)
    {
        if (tetra)
        {
            mEngine->Draw(tetra);
        }
    }
    mEngine->SetDefaultBlendState();

    mEngine->Draw(mSphere);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool Delaunay3DWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 's':
    case 'S':
        DoSearch();
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool Delaunay3DWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path != "")
    {
        mEnvironment.Insert(path + "/Samples/Geometrics/Delaunay3D/Data/");
        return true;
    }
    else
    {
        return false;
    }
}

bool Delaunay3DWindow3::CreateScene()
{
#if 1
    // Randomly generated points in the cube [-1,1]^3.  (0,0,0) is added to
    // ensure the hull contains the origin and the virtual trackball display
    // appears centered.
    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(-1.0f, 1.0f);
    mVertices.resize(128);
    for (auto& v : mVertices)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            v[j] = rnd(mte);
        }
    }
    mVertices[0] = Vector3<float>::Zero();
#endif

#if 0
    // A cube with 3x3x3 points.
    mVertices.resize(27);
    for (int32_t z = 0, i = 0; z < 3; ++z)
    {
        float fz = z - 1.0f;
        for (int32_t y = 0; y < 3; ++y)
        {
            float fy = y - 1.0f;
            for (int32_t x = 0; x < 3; ++x, ++i)
            {
                float fx = x - 1.0f;
                mVertices[i] = Vector3<float>(fx, fy, fz);
            }
        }
    }
#endif

#if 0
    // Some pathological examples (needle-like configurations).
    //std::string filename = "data1.txt";
    std::string filename = "data2.txt";
    //std::string filename = "data3.txt";

    std::string path = mEnvironment.GetPath(filename);
    if (path == "")
    {
        LogError("Cannot find file " + filename + ".");
        return false;
    }

    std::ifstream input(path);
    int32_t numVertices;
    input >> numVertices;
    mVertices.resize(numVertices);
    for (int32_t i = 0; i < numVertices; ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            input >> mVertices[i][j];
        }
    }
#endif

    Vector3<float> vmin, vmax;
    ComputeExtremes((int32_t)mVertices.size(), &mVertices[0], vmin, vmax);
    for (int32_t j = 0; j < 3; ++j)
    {
        mRandom[j] = std::uniform_real_distribution<float>(vmin[j], vmax[j]);
    }

    mDelaunay(mVertices);
    mInfo.initialTetrahedron = std::numeric_limits<size_t>::max();
    mInfo.finalTetrahedron = 0;

    mWireTetra.resize(mDelaunay.GetNumTetrahedra());
    mSolidTetra.resize(mDelaunay.GetNumTetrahedra());

    mScene = std::make_shared<Node>();
    CreateSphere();

    mIBuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 4, sizeof(uint32_t));
    auto* indices = mIBuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 3;  indices[5] = 1;
    indices[6] = 0;  indices[7] = 2;  indices[8] = 3;
    indices[9] = 3;  indices[10] = 2;  indices[11] = 1;

    mVCEffect = std::make_shared<VertexColorEffect>(mProgramFactory);

    for (size_t j = 0; j < mDelaunay.GetNumTetrahedra(); ++j)
    {
        CreateTetra(j);
    }
    mPVWMatrices.Subscribe(mWireTetra[0]->worldTransform, mVCEffect->GetPVWMatrixConstant());

    mTrackBall.Attach(mScene);
    mTrackBall.Update();
    return true;
}

void Delaunay3DWindow3::CreateSphere()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);

    mSphere = mf.CreateSphere(8, 8, 0.025f);
    std::shared_ptr<VertexColorEffect> effect =
        std::make_shared<VertexColorEffect>(mProgramFactory);
    mSphere->SetEffect(effect);

    // Move the sphere offscreen initially.
    mSphere->localTransform.SetTranslation(0.0f, 0.0f, -1000.0f);
    mSphere->Update();
    mPVWMatrices.Subscribe(mSphere->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mSphere);
}

void Delaunay3DWindow3::CreateTetra(size_t index)
{
    std::vector<int32_t> const& dindices = mDelaunay.GetIndices();

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    std::shared_ptr<VertexBuffer> vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (size_t j = 0; j < 4; ++j)
    {
        vertex[j].position = mVertices[dindices[4 * index + j]];
        vertex[j].color = mLightGray;
    }

    mWireTetra[index] = std::make_shared<Visual>(vbuffer, mIBuffer, mVCEffect);
    mScene->AttachChild(mWireTetra[index]);
}

void Delaunay3DWindow3::SetAllTetraWire()
{
    for (size_t i = 0; i < mSolidTetra.size(); ++i)
    {
        if (mSolidTetra[i])
        {
            mWireTetra[i] = mSolidTetra[i];
            mSolidTetra[i] = nullptr;
        }

        auto const& vbuffer = mWireTetra[i]->GetVertexBuffer();
        Vertex* vertex = vbuffer->Get<Vertex>();
        for (int32_t j = 0; j < 4; ++j)
        {
            vertex[j].color = mLightGray;
        }
        mEngine->Update(vbuffer);
    }
}

void Delaunay3DWindow3::SetTetraSolid(size_t index, Vector4<float> const& color)
{
    if (mWireTetra[index])
    {
        mSolidTetra[index] = mWireTetra[index];
        mWireTetra[index] = nullptr;
    }

    auto const& vbuffer = mSolidTetra[index]->GetVertexBuffer();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (int32_t j = 0; j < 4; ++j)
    {
        vertex[j].color = color;
    }
    mEngine->Update(vbuffer);
}

void Delaunay3DWindow3::SetLastTetraSolid(Vector4<float> const& color,
    Vector4<float> const& oppositeColor)
{
    size_t index = mInfo.finalTetrahedron;
    int32_t vOpposite = mInfo.finalV[3];

    if (mWireTetra[index])
    {
        mSolidTetra[index] = mWireTetra[index];
        mWireTetra[index] = nullptr;
    }

    auto const& vbuffer = mSolidTetra[index]->GetVertexBuffer();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (int32_t j = 0; j < 4; ++j)
    {
        if (j != vOpposite)
        {
            vertex[j].color = color;
        }
        else
        {
            vertex[j].color = oppositeColor;
        }
    }
    mEngine->Update(vbuffer);
}

void Delaunay3DWindow3::DoSearch()
{
    SetAllTetraWire();

    // Generate random point in AABB of data set.
    Vector3<float> point{};
    for (int32_t j = 0; j < 3; ++j)
    {
        point[j] = mRandom[j](mRandomGenerator);
    }

    // Move sphere to this location.
    mSphere->localTransform.SetTranslation(point);
    mSphere->Update();
    mPVWMatrices.Update();

    mInfo.initialTetrahedron = mInfo.finalTetrahedron;
    size_t found = mDelaunay.GetContainingTetrahedron(point, mInfo);
    if (found != std::numeric_limits<size_t>::max())
    {
        mInfo.initialTetrahedron = mInfo.finalTetrahedron;

        // Make all tetra on the path solid.
        for (size_t i = 0; i < mInfo.numPath; ++i)
        {
            size_t index = mInfo.path[i];
            float red, blue;
            if (mInfo.numPath > 1)
            {
                red = i / (mInfo.numPath - 1.0f);
                blue = 1.0f - red;
            }
            else
            {
                red = 1.0f;
                blue = 0.0f;
            }
            SetTetraSolid(index, { red, 0.0f, blue, 0.5f });
        }
    }
    else
    {
        // The point is outside the convex hull.  Change the wireframe
        // color for the last visited face in the search path.
        SetLastTetraSolid({ 0.0f, 1.0f, 0.0f, 0.5f }, { 0.0f, 0.25f, 0.0f, 0.5f });
    }
}
