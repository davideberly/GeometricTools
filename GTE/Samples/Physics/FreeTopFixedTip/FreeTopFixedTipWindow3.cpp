// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "FreeTopFixedTipWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>

FreeTopFixedTipWindow3::FreeTopFixedTipWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mMaxPhi(0.0f),
    mLastUpdateTime(mMotionTimer.GetSeconds())
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();
    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 2.0f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();
}

void FreeTopFixedTipWindow3::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    double time = mMotionTimer.GetSeconds();
    if (30.0 * (time - mLastUpdateTime) >= 1.0)
    {
        mLastUpdateTime = time;
#if !defined(FREE_TOP_FIXED_TIP_SINGLE_STEP)
        PhysicsTick();
#endif
        mTrackBall.Update();
        GraphicsTick();
    }
}

bool FreeTopFixedTipWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':
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

    case 'i':
    case 'I':
        InitializeModule();
        return true;

#if defined(FREE_TOP_FIXED_TIP_SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool FreeTopFixedTipWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Wood.png",
        "TopTexture.png"
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

void FreeTopFixedTipWindow3::InitializeModule()
{
    mModule.gravity = 10.0f;
    mModule.mass = 1.0f;
    mModule.length = 8.0f;
    mModule.inertia1 = 1.0f;
    mModule.inertia3 = 2.0f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float theta = 0.0f;
    float phi = 0.001f;
    float psi = 0.0f;
    float angVel1 = 1.0f;
    float angVel2 = 0.0f;
    float angVel3 = 10.0f;
    mModule.Initialize(time, deltaTime, theta, phi, psi, angVel1, angVel2, angVel3);
    mMaxPhi = static_cast<float>(GTE_C_HALF_PI - std::atan(2.0 / 3.0));
}

void FreeTopFixedTipWindow3::CreateScene()
{
    // scene -+--- floor
    //        |
    //        +--- vertical axis
    //        |
    //        +--- top root ---+--- top
    //                         |
    //                         +--- top axis

    InitializeModule();

    mScene = std::make_shared<Node>();
    mTopRoot = std::make_shared<Node>();
    mScene->AttachChild(mTopRoot);
    CreateFloor();
    CreateAxisVertical();
    CreateTop();
    CreateAxisTop();
    mTrackBall.Attach(mScene);
    mTrackBall.Update();
}

void FreeTopFixedTipWindow3::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateRectangle(2, 2, 32.0f, 32.0f);

    auto texture = WICFileIO::Load(mEnvironment.GetPath("Wood.png"), true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::CLAMP, SamplerState::Mode::CLAMP);

    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mScene->AttachChild(visual);
}

void FreeTopFixedTipWindow3::CreateAxisVertical()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    auto vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 4.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);

    Vector4<float> black{ 0.0f, 0.0f, 0.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, black);

    auto visual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mScene->AttachChild(visual);
}

void FreeTopFixedTipWindow3::CreateTop()
{
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    auto visual = mf.CreateCylinderOpen(32, 32, 1.0f, 2.0f);
    visual->localTransform.SetTranslation(0.0f, 0.0f, 1.0f);

    // Adjust the shape.
    auto const& vbuffer = visual->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float>& pos = vertices[i].position;
        float z = pos[2] + 1.0f;
        float r = 0.75f * (z >= 1.5f ? 4.0f - 2.0f * z : z / 1.5f);
        float multiplier = r / std::sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
        pos[0] *= multiplier;
        pos[1] *= multiplier;
        vertices[i].tcoord *= 4.0f;
    }

    auto texture = WICFileIO::Load(mEnvironment.GetPath("TopTexture.png"), true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mTopRoot->AttachChild(visual);
}

void FreeTopFixedTipWindow3::CreateAxisTop()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 2);
    auto vertices = vbuffer->Get<Vector3<float>>();
    vertices[0] = { 0.0f, 0.0f, 0.0f };
    vertices[1] = { 0.0f, 0.0f, 4.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, 1);

    Vector4<float> white{ 1.0f, 1.0f, 1.0f, 1.0f };
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, white);

    auto visual = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    visual->SetEffect(effect);
    mPVWMatrices.Subscribe(visual->worldTransform, effect->GetPVWMatrixConstant());
    mVisuals.push_back(visual);
    mTopRoot->AttachChild(visual);
}

void FreeTopFixedTipWindow3::PhysicsTick()
{
    // Stop the simulation when the top edge reaches the ground.
    if (mModule.GetPhi() >= mMaxPhi)
    {
        // EXERCISE.  Instead of stopping the top, maintain its phi value at
        // mMaxPhi so that the top continues to roll on the ground.  In
        // addition, arrange for the top to slow down while rolling on the
        // ground, eventually coming to a stop.
        return;
    }

    // Move the top
    mModule.Update();
    mTopRoot->localTransform.SetRotation(mModule.GetBodyAxes());
    mTopRoot->Update();
    mPVWMatrices.Update();
}

void FreeTopFixedTipWindow3::GraphicsTick()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    for (auto const& visual : mVisuals)
    {
        mEngine->Draw(visual);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}
