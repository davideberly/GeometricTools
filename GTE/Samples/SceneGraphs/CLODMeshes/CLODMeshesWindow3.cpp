// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.02.24

#include "CLODMeshesWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/CLODMeshCreator.h>
#include <Graphics/Texture2Effect.h>

CLODMeshesWindow3::CLODMeshesWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mWireState(),
    mScene{},
    mTrnNode{},
    mCLODMesh{},
    mTextColor{ 0.0f, 0.0f, 0.0f, 1.0f }
{
    if (!SetEnvironment())
    {
        parameters.created = false;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();

    // Center-and-fit scene in the view frustum.
    mScene->Update();
    Vector3<float> center = mScene->worldBound.GetCenter();
    float radius = mScene->worldBound.GetRadius();
    mTrnNode->localTransform.SetTranslation(-center);
    Vector3<float> camDVector{ -1.0f, 0.0f, 0.0f };
    Vector3<float> camUVector{ 0.0f, 0.0f, 1.0f };
    Vector3<float> camPosition = -3.0f * radius * camDVector;
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.001f, 0.001f,
        { camPosition[0], camPosition[1], camPosition[2] },
        { camDVector[0], camDVector[1], camDVector[2] },
        { camUVector[0], camUVector[1], camUVector[2] });
    mScene->Update();

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void CLODMeshesWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    UpdateCLODMesh();

    mEngine->ClearBuffers();
    mEngine->Draw(mCLODMesh[0]);
    mEngine->Draw(mCLODMesh[1]);

    uint32_t numTri0 = mCLODMesh[0]->GetIndexBuffer()->GetNumActiveElements() / 3;
    uint32_t numTri1 = mCLODMesh[1]->GetIndexBuffer()->GetNumActiveElements() / 3;
    std::string message = "triangles0 = " + std::to_string(numTri0) +
        ", triangles1 = " + std::to_string(numTri1);
    mEngine->Draw(8, 24, mTextColor, message);

    mEngine->Draw(8, mYSize - 8, mTextColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool CLODMeshesWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mEngine->GetRasterizerState() == mWireState)
        {
            mEngine->SetDefaultRasterizerState();
        }
        else
        {
            mEngine->SetRasterizerState(mWireState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool CLODMeshesWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/SceneGraphs/CLODMeshes/Data/");
    std::vector<std::string> inputs =
    {
        "FunctionX64Y64R8.png",
        "Magician.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
            return false;
        }
    }

    return true;
}

void CLODMeshesWindow3::CreateScene()
{
    std::string path = mEnvironment.GetPath("FunctionX64Y64R8.png");
    auto heightTexture = WICFileIO::Load(path, false);
    auto* heights = heightTexture->Get<uint8_t>();
    std::vector<Vertex> inVertices(4096);
    for (size_t y = 0, i = 0; y < 64; ++y)
    {
        float py = -1.0f + (float)y / 32.0f;
        float ty = (float)y / 64.0f;
        for (size_t x = 0; x < 64; ++x, ++i)
        {
            float px = -1.0f + (float)x / 32.0f;
            float tx = (float)x / 64.0f;
            inVertices[i].position[0] = px;
            inVertices[i].position[1] = py;
            inVertices[i].position[2] = (float)heights[i] / 255.0f;
            inVertices[i].tcoord[0] = tx;
            inVertices[i].tcoord[1] = ty;
        }
    }

    std::vector<int32_t> inIndices(23814);  // 3 * 2 * (64-1) * (64-1)
    for (int32_t i1 = 0, j = 0; i1 < 63; ++i1)
    {
        for (int32_t i0 = 0; i0 < 63; ++i0)
        {
            int32_t v0 = i0 + 64 * i1;
            int32_t v1 = v0 + 1;
            int32_t v2 = v1 + 64;
            int32_t v3 = v0 + 64;
            inIndices[j++] = v0;
            inIndices[j++] = v1;
            inIndices[j++] = v2;
            inIndices[j++] = v0;
            inIndices[j++] = v2;
            inIndices[j++] = v3;
        }
    }

    std::vector<Vertex> outVertices{};
    std::vector<int32_t> outIndices{};
    std::vector<CLODCollapseRecord> records{};

    CLODMeshCreator<Vertex> creator{};
    creator(inVertices, inIndices, outVertices, outIndices, records);

    // The texture image is shared by the CLOD meshes.
    path = mEnvironment.GetPath("Magician.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();

    // The vertex buffer is shared by the CLOD meshes.
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    uint32_t numVertices = static_cast<uint32_t>(outVertices.size());
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i] = outVertices[i];
    }

    // The CLOD meshes cannot share the same index buffer because each
    // mesh modifies the indices differently during changes in level of
    // detail. As per current GTE graphics engine design, the effect
    // cannot be shared because each has its own model-to-world transform.
    uint32_t numIndices = static_cast<uint32_t>(outIndices.size());
    uint32_t numTriangles = numIndices / 3;
    std::shared_ptr<IndexBuffer> ibuffer{};
    int32_t* indices = nullptr;
    std::shared_ptr<Texture2Effect> effect{};

    ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH,
        numTriangles, sizeof(int32_t));
    ibuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    indices = ibuffer->Get<int32_t>();
    for (uint32_t i = 0; i < numIndices; ++i)
    {
        indices[i] = outIndices[i];
    }

    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);

    mCLODMesh[0] = std::make_shared<CLODMesh>(records);
    //mCLODMesh[0]->localTransform.SetTranslation(-150.0f, 0.0f, 0.0f);
    mCLODMesh[0]->localTransform.SetTranslation(0.0f, -2.0f, 0.0f);
    mCLODMesh[0]->SetVertexBuffer(vbuffer);
    mCLODMesh[0]->SetIndexBuffer(ibuffer);
    mCLODMesh[0]->SetEffect(effect);
    mCLODMesh[0]->UpdateModelBound();
    mPVWMatrices.Subscribe(mCLODMesh[0]);

    ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH,
        numTriangles, sizeof(int32_t));
    ibuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    indices = ibuffer->Get<int32_t>();
    for (uint32_t i = 0; i < numIndices; ++i)
    {
        indices[i] = outIndices[i];
    }

    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP,
        SamplerState::Mode::CLAMP);

    mCLODMesh[1] = std::make_shared<CLODMesh>(records);
    //mCLODMesh[1]->localTransform.SetTranslation(150.0f, -100.0f, 0.0f);
    mCLODMesh[1]->localTransform.SetTranslation(0.0f, 2.0f, 0.0f);
    mCLODMesh[1]->SetVertexBuffer(vbuffer);
    mCLODMesh[1]->SetIndexBuffer(ibuffer);
    mCLODMesh[1]->SetEffect(effect);
    mCLODMesh[1]->UpdateModelBound();
    mPVWMatrices.Subscribe(mCLODMesh[1]);

    // The scene graph is
    // scene
    //     trnNode
    //         clodMesh0
    //         clodMesh1
    mScene = std::make_shared<Node>();
    mTrnNode = std::make_shared<Node>();
    mScene->AttachChild(mTrnNode);
    mTrnNode->AttachChild(mCLODMesh[0]);
    mTrnNode->AttachChild(mCLODMesh[1]);
    mTrackBall.Attach(mScene);
}

void CLODMeshesWindow3::UpdateCLODMesh()
{
    // the camera direction controls the triangle quantities. A nonlinear
    // drop-off is used.
    for (size_t i = 0; i < 2; ++i)
    {
        Vector3<float> diff = mCLODMesh[i]->worldBound.GetCenter() -
            HProject(mCamera->GetPosition());

        float depth = Dot(HProject(mCamera->GetDVector()), diff);
        int targetRecord;
        if (depth <= mCamera->GetDMin())
        {
            targetRecord = 0;
        }
        else if (depth >= mCamera->GetDMax())
        {
            targetRecord = mCLODMesh[i]->GetNumRecords() - 1;
        }
        else
        {
            float dmin = mCamera->GetDMin();
            float dmax = mCamera->GetDMax();
            float ratio = std::pow((depth - dmin) / (dmax - dmin), 0.5f);
            targetRecord = static_cast<int32_t>(
                (mCLODMesh[i]->GetNumRecords() - 1) * ratio);
        }

        if (mCLODMesh[i]->SetTargetRecord(targetRecord))
        {
            mEngine->Update(mCLODMesh[i]->GetIndexBuffer());
        }
    }
}
