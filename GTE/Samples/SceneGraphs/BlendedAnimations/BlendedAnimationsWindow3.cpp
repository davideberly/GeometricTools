// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BlendedAnimationsWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>

BlendedAnimationsWindow3::BlendedAnimationsWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mUpArrowPressed(false),
    mShiftPressed(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    std::string gtePath = mEnvironment.GetGTEPath();
    LogAssert(gtePath != "", "The path to the GTE folder is unknown.");
    std::string rootPath = gtePath + "/Samples/SceneGraphs/BlendedAnimations/Data/";
    mManager = std::make_unique<BipedManager>(rootPath, "Biped", mProgramFactory, mUpdater);

    // Set animation information.
    int32_t idleWalkCount = 100;
    int32_t walkCount = 10;
    int32_t walkRunCount = 100;
    mLastAnimTime = 0.0;

    // The idle head turning occurs too frequently (frequency = 1 in the
    // original model).  Reduce the turning by half.
    mManager->SetIdle(0.5, 0.0);

    // The walk and run cycles must be aligned properly for blending.  A
    // phase of 0.2 for the run cycle aligns the biped feet.
    mManager->SetRun(1.0, 0.2);

    // The initial state is 'idle'.
    mManager->Initialize(idleWalkCount, walkCount, walkRunCount);

    CreateScene();

    Vector3<float> dir{ 1.0f, 1.0f, -1.0f }, up{ 0.5f, 0.5f, 1.0f };
    Normalize(dir);
    Normalize(up);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.01f, 0.01f,
        { -60.0f, -60.0f, 90.0f }, { dir[0], dir[1], dir[2] }, { up[0], up[1], up[2] });
    mPVWMatrices.Update();
}

void BlendedAnimationsWindow3::OnIdle()
{
    mTimer.Measure();

    mCurrAnimTime = mAnimTimer.GetSeconds();
    double deltaAnimTime = mCurrAnimTime - mLastAnimTime;
    if (deltaAnimTime >= 1.0 / 120.0)
    {
        Update();
        mLastAnimTime = mCurrAnimTime;
    }

    mEngine->ClearBuffers();
    for (auto const& mesh : mMeshes)
    {
        mEngine->Draw(mesh);
    }

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    mEngine->Draw(8, 24, textColor, "Press UP-ARROW to transition from idle to walk.");
    mEngine->Draw(8, 48, textColor,  "Press SHIFT-UP-ARROW to transition from walk to run.");
    mEngine->Draw(128, mYSize - 8, textColor, "time = " + std::to_string(mCurrAnimTime));
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool BlendedAnimationsWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

bool BlendedAnimationsWindow3::OnKeyDown(int32_t key, int32_t, int32_t)
{
    if (key == KEY_UP)
    {
        mUpArrowPressed = true;
    }
    else if (key == KEY_SHIFT)
    {
        mShiftPressed = true;
    }

    // Window3::OnKeyDown is not called to prevent camera motion.
    return true;
}

bool BlendedAnimationsWindow3::OnKeyUp(int32_t key, int32_t, int32_t)
{
    if (key == KEY_UP)
    {
        mUpArrowPressed = false;
    }
    else if (key == KEY_SHIFT)
    {
        mShiftPressed = false;
    }

    // Window3::OnKeyUp is not called to prevent camera motion.
    return true;
}

bool BlendedAnimationsWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    mEnvironment.Insert(path + "/Samples/SceneGraphs/BlendedAnimations/Data/");
    if (mEnvironment.GetPath("Grating.png") == "")
    {
        LogError("Cannot find file Grating.png.");
        return false;
    }

    return true;
}

void BlendedAnimationsWindow3::CreateScene()
{
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    mScene = std::make_shared<Node>();
    mScene->AttachChild(mManager->GetRoot());

    // Create a floor to walk on.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mFloor = mf.CreateRectangle(2, 2, 1024.0f, 2048.0f);
    mFloor->name = "Floor";
    mScene->AttachChild(mFloor);
    auto const& vbuffer = mFloor->GetVertexBuffer();
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    uint32_t numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord[0] *= 64.0f;
        vertices[i].tcoord[1] *= 256.0f;
    }

    std::string textureName = mEnvironment.GetPath("Grating.png");
    auto texture = WICFileIO::Load(textureName, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mFloor->SetEffect(effect);

    mPVWMatrices.Subscribe(mFloor->worldTransform, effect->GetPVWMatrixConstant());
    for (auto const& subscriber : mManager->GetSubscribers())
    {
        mPVWMatrices.Subscribe(subscriber.first->worldTransform, subscriber.second);
    }

    GetMeshes(mScene);

    mTrackBall.Attach(mScene);
    mTrackBall.Update(mCurrAnimTime);
}

void BlendedAnimationsWindow3::GetMeshes(
    std::shared_ptr<Spatial> const& object)
{
    Visual* mesh = dynamic_cast<Visual*>(object.get());
    if (mesh)
    {
        mMeshes.push_back(mesh);
        return;
    }

    Node* node = dynamic_cast<Node*>(object.get());
    if (node)
    {
        for (int32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            auto const& child = node->GetChild(i);
            if (child)
            {
                GetMeshes(child);
            }
        }
    }
}

void BlendedAnimationsWindow3::Update()
{
    // Animate the biped.
    mManager->Update(mUpArrowPressed, mShiftPressed);
    mScene->Update(mCurrAnimTime);

    // Give the impression the floor is moving by translating the texture
    // coordinates.  For this demo, the texture coordinates are modified in
    // the vertex buffer.  Generally, you could write a vertex shader with
    // time and velocity as inputs, and then compute the new texture
    // coordinates in the shader.
    float adjust = mManager->GetSpeed() / 16.0f;
    std::shared_ptr<VertexBuffer> vbuffer = mFloor->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    Vertex* vertex = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertex[i].tcoord[1] -= adjust;
    }
    mEngine->Update(vbuffer);
}
