// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "VisualizeMarchingCubesWindow3.h"

VisualizeMarchingCubesWindow3::VisualizeMarchingCubesWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mTextColor{ 0.0f, 0.0f, 0.0f, 0.0f },
    mEnvironment{},
    mImage(2, 2, 2),
    mExtractor(mImage),
    mColors  // has Extractor::Topology::maxTriangles = 5 elements
    {
        Vector4<float>(0.5f, 0.0f, 0.0f, 1.0f),
        Vector4<float>(0.0f, 0.5f, 0.0f, 1.0f),
        Vector4<float>(0.0f, 0.0f, 0.5f, 1.0f),
        Vector4<float>(0.5f, 0.5f, 0.0f, 1.0f),
        Vector4<float>(0.5f, 0.0f, 0.5f, 1.0f)
    },
    mScene{},
    mNoCullState{},
    mNoCullWireState{},
    mEffect{},
    mBox{},
    mMesh{},
    mCurrentEntry(1),
    mCurrentString("")
{
    // Disable culling.
    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::NONE;
    mNoCullState->fill = RasterizerState::SOLID;
    mEngine->SetRasterizerState(mNoCullState);

    // Enable wireframe (when requested).
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cull = RasterizerState::NONE;
    mNoCullWireState->fill = RasterizerState::WIREFRAME;

    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.01f, 0.001f,
        { 2.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });

    GetCurrentString();
    CreateScene();
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void VisualizeMarchingCubesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mBox);
    mEngine->Draw(mMesh);
    mEngine->Draw(8, 24, mTextColor, mCurrentString);
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool VisualizeMarchingCubesWindow3::OnCharPress(std::uint8_t key, std::int32_t x, std::int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mNoCullState)
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;

    case '+':
    case '=':
        if (mCurrentEntry < 254)
        {
            ++mCurrentEntry;
        }
        else
        {
            mCurrentEntry = 1;
        }
        GetCurrentString();
        CreateMesh();
        return true;

    case '-':
    case '_':
        if (mCurrentEntry > 1)
        {
            --mCurrentEntry;
        }
        else
        {
            mCurrentEntry = 254;
        }
        GetCurrentString();
        CreateMesh();
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

void VisualizeMarchingCubesWindow3::CreateScene()
{
    // Create the root node. Set the translation so that the trackball rotates
    // about the center of the box.
    mScene = std::make_shared<Node>();
    mScene->localTransform.SetTranslation(-0.5f, -0.5f, -0.5f);
    mTrackBall.Attach(mScene);

    // Create a wireframe box.
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 8);
    auto* vertices = vbuffer->Get<Vertex>();
    for (std::size_t i = 0; i < 8; ++i)
    {
        vertices[i].position = Vector3<float>(
            static_cast<float>(i & 1),
            static_cast<float>((i & 2) >> 1),
            static_cast<float>((i & 4) >> 2));
    }
    vertices[0].color = Vector4<float>(1.0f, 0.0f, 0.0f, 1.0f);
    vertices[1].color = Vector4<float>(0.0f, 1.0f, 0.0f, 1.0f);
    vertices[2].color = Vector4<float>(0.0f, 0.0f, 1.0f, 1.0f);
    vertices[3].color = Vector4<float>(0.25f, 0.25f, 0.25f, 1.0f);
    vertices[4].color = Vector4<float>(0.0f, 1.0f, 1.0f, 1.0f);
    vertices[5].color = Vector4<float>(1.0f, 0.0f, 1.0f, 1.0f);
    vertices[6].color = Vector4<float>(1.0f, 1.0f, 0.0f, 1.0f);
    vertices[7].color = Vector4<float>(0.75f, 0.75f, 0.75f, 1.0f);

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 12, sizeof(std::uint32_t));
    auto* indices = ibuffer->Get<std::uint32_t>();
    indices[0] = 0;  indices[1] = 1;
    indices[2] = 1;  indices[3] = 3;
    indices[4] = 3;  indices[5] = 2;
    indices[6] = 2;  indices[7] = 0;
    indices[8] = 4;  indices[9] = 5;
    indices[10] = 5;  indices[11] = 7;
    indices[12] = 7;  indices[13] = 6;
    indices[14] = 6;  indices[15] = 4;
    indices[16] = 0;  indices[17] = 4;
    indices[18] = 1;  indices[19] = 5;
    indices[20] = 2;  indices[21] = 6;
    indices[22] = 3;  indices[23] = 7;

    mEffect = std::make_shared<VertexColorEffect>(mProgramFactory);

    mBox = std::make_shared<Visual>(vbuffer, ibuffer, mEffect);
    mPVWMatrices.Subscribe(mBox);
    mScene->AttachChild(mBox);

    // Create a mesh for the extracted surface. The mesh has a list of
    // triangles, each a separate color, so some vertices are duplicated.
    // The number of vertices is maxVertices = 3 * maxTriangles = 15.
    std::uint32_t maxVertices = 15;
    vbuffer = std::make_shared<VertexBuffer>(vformat, maxVertices);
    vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());

    std::uint32_t maxTriangles = 5;
    ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, maxTriangles, sizeof(std::uint32_t));
    ibuffer->SetUsage(Resource::DYNAMIC_UPDATE);
    std::memset(ibuffer->GetData(), 0, ibuffer->GetNumBytes());

    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, mEffect);
    mPVWMatrices.Subscribe(mMesh);
    mScene->AttachChild(mMesh);
    CreateMesh();
}

void VisualizeMarchingCubesWindow3::CreateMesh()
{
    std::array<float, 8> F{};
    for (std::size_t i = 0, mask = 1; i < 8; ++i, mask <<= 1)
    {
        F[i] = (mCurrentEntry & mask ? -1.0f : 2.0f);
    }

    Extractor::Mesh mesh{};
    mExtractor.Extract(0.0f, 0.0f, F, mesh);

    auto const& vbuffer = mMesh->GetVertexBuffer();
    auto* vertices = vbuffer->Get<Vertex>();
    auto const& ibuffer = mMesh->GetIndexBuffer();
    auto* indices = ibuffer->Get<std::uint32_t>();
    for (std::uint32_t i = 0, k = 0; i < mesh.topology.numTriangles; ++i)
    {
        auto const& itriple = mesh.topology.itriple[i];
        for (std::uint32_t j = 0; j < 3; ++j, ++vertices, ++indices, ++k)
        {
            vertices->position = mesh.vertices[itriple[j]];
            vertices->color = mColors[i];
            *indices = k;
        }
    }
    vbuffer->SetNumActiveElements(3 * mesh.topology.numTriangles);
    ibuffer->SetNumActivePrimitives(mesh.topology.numTriangles);
    mEngine->Update(vbuffer);
    mEngine->Update(ibuffer);

    mTrackBall.Update();
}

void VisualizeMarchingCubesWindow3::GetCurrentString()
{
    mCurrentString = std::to_string(mCurrentEntry) + ": "
        + Extractor::GetConfigurationType(mCurrentEntry);
}

