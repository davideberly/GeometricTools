// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "ConvexHull3DWindow3.h"
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ConvexHull3.h>
#include <random>

ConvexHull3DWindow3::ConvexHull3DWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mFileQuantity(46),
    mCurrentFile(1)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 5000.0f, 0.1f, 0.01f,
        { 0.0f, 0.0f, -4.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f });

    mEffect = std::make_shared<VertexColorEffect>(mProgramFactory);
    if (!LoadData())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->cull = RasterizerState::Cull::NONE;
    mWireState->fill = RasterizerState::Fill::WIREFRAME;
}

void ConvexHull3DWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    if (mMesh)
    {
        mEngine->Draw(mMesh);
    }

    std::array<float, 4> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    if (mMessage != "")
    {
        mEngine->Draw(8, 16, textColor, mMessage);
    }

    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ConvexHull3DWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'd':  // Load a new data set.
    case 'D':
        if (++mCurrentFile == mFileQuantity)
        {
            mCurrentFile = 1;
        }

        LoadData();
        return true;

    case 'w':  // Toggle solid-wire mode.
    case 'W':
        if (mWireState == mEngine->GetRasterizerState())
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

bool ConvexHull3DWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path != "")
    {
        mEnvironment.Insert(path + "/Samples/Geometrics/ConvexHull3D/Data/");
        return true;
    }
    else
    {
        return false;
    }
}

bool ConvexHull3DWindow3::LoadData()
{
    std::string filename = "data";
    if (mCurrentFile < 10)
    {
        filename += "0";
    }
    filename += std::to_string(mCurrentFile) + ".txt";
    std::string path = mEnvironment.GetPath(filename);
    if (path == "")
    {
        return false;
    }

    std::ifstream input(path);
    if (!input)
    {
        return false;
    }

    Vector3<float> center{ 0.0f, 0.0f, 0.0f };
    uint32_t numVertices;
    input >> numVertices;
    std::vector<Vector3<float>> vertices(numVertices);
    for (auto& v : vertices)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            input >> v[j];
        }
        center += v;
    }
    input.close();
    center /= static_cast<float>(numVertices);

    float radius = 0.0f;
    for (auto const& v : vertices)
    {
        Vector3<float> diff = v - center;
        float length = Length(diff);
        if (length > radius)
        {
            radius = length;
        }
    }

    ConvexHull3<float> ch;
    ch(vertices, 0);
    if (numVertices < 4 || ch.GetDimension() < 3)
    {
        if (mMesh)
        {
            mTrackBall.Detach(mMesh);
            mTrackBall.Update();
            mPVWMatrices.Unsubscribe(mMesh->worldTransform);
            mMesh = nullptr;
        }

        mMessage = "File = " + std::to_string(mCurrentFile) +
            " has intrinsic dimension " + std::to_string(ch.GetDimension());
        return false;
    }
#if defined(GTE_COLLECT_BSNUMBER_STATISTICS)
    std::cout << "max size = " << gte::gBSNumberMaxSize << std::endl;
#endif

    std::vector<size_t> const& hull = ch.GetHull();

    std::mt19937 mte;
    std::uniform_real_distribution<float> rnd(0.0f, 1.0f);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertex[i].position = vertices[i];
        vertex[i].color[0] = rnd(mte);
        vertex[i].color[1] = rnd(mte);
        vertex[i].color[2] = rnd(mte);
        vertex[i].color[3] = 1.0f;
    }

    uint32_t numPrimitives = static_cast<uint32_t>(hull.size() / 3);
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, numPrimitives, sizeof(int32_t));
    auto indices = ibuffer->Get<int32_t>();
    for (size_t i = 0; i < hull.size(); ++i)
    {
        indices[i] = static_cast<int32_t>(hull[i]);
    }

    // Update all information associated with the mesh transforms.
    if (mMesh)
    {
        mTrackBall.Detach(mMesh);
        mTrackBall.Update();
        mPVWMatrices.Unsubscribe(mMesh->worldTransform);
    }
    mMesh = std::make_shared<Visual>(vbuffer, ibuffer, mEffect);
    mMesh->localTransform.SetTranslation(-center);
    mMesh->worldTransform = mMesh->localTransform;
    mPVWMatrices.Subscribe(mMesh->worldTransform, mEffect->GetPVWMatrixConstant());

    // Move the camera for a centered view of the mesh.
    Vector4<float> camPosition = Vector4<float>{0.0f, 0.0f, 0.0f, 1.0f}
        - 2.5f*radius*mCamera->GetDVector();
    mCamera->SetPosition(camPosition);

    // Update the message for display.
    mMessage =
        "File = " + std::to_string(mCurrentFile) + " , " +
        "Vertices = " + std::to_string(numVertices) + " , " +
        "Triangles =" + std::to_string(numPrimitives);

    mTrackBall.Attach(mMesh);
    mTrackBall.Update();
    mPVWMatrices.Update();
    return true;
}
