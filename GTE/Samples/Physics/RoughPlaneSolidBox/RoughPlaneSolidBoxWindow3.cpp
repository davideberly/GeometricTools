// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#include "RoughPlaneSolidBoxWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/VertexColorEffect.h>

//#define SINGLE_STEP

RoughPlaneSolidBoxWindow3::RoughPlaneSolidBoxWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mDoUpdate(true),
    mPhysicsTimer{},
    mLastPhysicsTime(0.0),
    mCurrPhysicsTime(0.0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::WIREFRAME;

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.001f, 0.001f,
        { 17.695415f, 0.0f, 6.4494629f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });

    InitializeModule();
    CreateScene();
}

void RoughPlaneSolidBoxWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#ifndef SINGLE_STEP
    PhysicsTick();
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool RoughPlaneSolidBoxWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
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

    case 'r':  // restart
    case 'R':
        InitializeModule();
        MoveBox();
        return true;

#ifdef SINGLE_STEP
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool RoughPlaneSolidBoxWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    std::vector<std::string> inputs =
    {
        "Gravel.png",
        "Metal.png"
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

void RoughPlaneSolidBoxWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();
    CreateGround();
    CreateRamp();
    CreateBox();

    AxisAngle<3, float> aa(Vector3<float>{ 0.0f, 0.0f, 1.0f }, 0.661917f);
    mScene->localTransform.SetRotation(aa);

    mTrackBall.Attach(mScene);
}

void RoughPlaneSolidBoxWindow3::CreateGround()
{
    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    MeshFactory mf{};
    mf.SetVertexFormat(vformat);
    mGround = mf.CreateRectangle(2, 2, 32.0f, 32.0f);

    // Change the texture repeat.
    auto const& vbuffer = mGround->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    auto* vertices = vbuffer->Get<VertexPT>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord *= 8.0f;
    }

    std::string gravelFile = mEnvironment.GetPath("Gravel.png");
    auto texture = WICFileIO::Load(gravelFile, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mGround->SetEffect(effect);
    mPVWMatrices.Subscribe(mGround);
    mScene->AttachChild(mGround);
}

void RoughPlaneSolidBoxWindow3::CreateRamp()
{
    float x = 8.0f;
    float y = 8.0f;
    float z = y * std::tan(static_cast<float>(mModule.angle));

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 6);
    auto* vertices = vbuffer->Get<VertexPT>();
    vertices[0].position = { -x, 0.0f, 0.0f };
    vertices[1].position = { +x, 0.0f, 0.0f };
    vertices[2].position = { -x, y, 0.0f };
    vertices[3].position = { +x, y, 0.0f };
    vertices[4].position = { -x, y, z };
    vertices[5].position = { +x, y, z };
    vertices[0].tcoord = { 0.25f, 0.0f };
    vertices[1].tcoord = { 0.75f, 0.0f };
    vertices[2].tcoord = { 0.0f, 1.0f };
    vertices[3].tcoord = { 1.0f, 1.0f };
    vertices[4].tcoord = { 0.25f, 1.0f };
    vertices[5].tcoord = { 0.75f, 1.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IPType::IP_TRIMESH, 6,
        sizeof(uint32_t));
    ibuffer->SetTriangle(0, 0, 1, 4);
    ibuffer->SetTriangle(1, 1, 5, 4);
    ibuffer->SetTriangle(2, 0, 4, 2);
    ibuffer->SetTriangle(3, 1, 3, 5);
    ibuffer->SetTriangle(4, 3, 2, 4);
    ibuffer->SetTriangle(5, 3, 4, 5);

    std::string metalFile = mEnvironment.GetPath("Metal.png");
    auto texture = WICFileIO::Load(metalFile, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mRamp = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mRamp);
    mScene->AttachChild(mRamp);
}

void RoughPlaneSolidBoxWindow3::CreateBox()
{
    mBox = std::make_shared<Node>();
    mScene->AttachChild(mBox);

    float xExtent = static_cast<float>(mModule.xLocExt);
    float yExtent = static_cast<float>(mModule.yLocExt);
    float zExtent = static_cast<float>(mModule.zLocExt);

    VertexFormat vformat{};
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::COLOR, DF_R32G32B32A32_FLOAT, 0);

    MeshFactory mf{};
    mf.SetVertexFormat(vformat);
    Matrix3x3<float> rotate{};
    Vector3<float> translate{};
    std::shared_ptr<VertexBuffer> vbuffer{};
    VertexPC* vertices = nullptr;
    std::shared_ptr<VertexColorEffect> effect{};

    // +z face
    Vector4<float> red{ 1.0f, 0.0f, 0.0f, 1.0f };
    mBoxFace[0] = mf.CreateRectangle(2, 2, xExtent, yExtent);
    translate = { 0.0f, 0.0f, zExtent };
    rotate.SetCol(0, { 1.0f, 0.0f, 0.0f });
    rotate.SetCol(1, { 0.0f, 1.0f, 0.0f });
    rotate.SetCol(2, { 0.0f, 0.0f, 1.0f });
    mBoxFace[0]->localTransform.SetTranslation(translate);
    mBoxFace[0]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[0]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = red;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[0]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[0]);
    mBox->AttachChild(mBoxFace[0]);

    // -z face
    Vector4<float> darkRed{ 0.5f, 0.0f, 0.0f, 1.0f };
    mBoxFace[1] = mf.CreateRectangle(2, 2, yExtent, xExtent);
    translate = { 0.0f, 0.0f, -zExtent };
    rotate.SetCol(0, { 0.0f, 1.0f, 0.0f });
    rotate.SetCol(1, { 1.0f, 0.0f, 0.0f });
    rotate.SetCol(2, { 0.0f, 0.0f, -1.0f });
    mBoxFace[1]->localTransform.SetTranslation(translate);
    mBoxFace[1]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[1]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = darkRed;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[1]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[1]);
    mBox->AttachChild(mBoxFace[1]);

    // +y face
    Vector4<float> green{ 0.0f, 1.0f, 0.0f, 1.0f };
    mBoxFace[2] = mf.CreateRectangle(2, 2, zExtent, xExtent);
    translate = { 0.0f, yExtent, 0.0f };
    rotate.SetCol(0, { 0.0f, 0.0f, 1.0f });
    rotate.SetCol(1, { 1.0f, 0.0f, 0.0f });
    rotate.SetCol(2, { 0.0f, 1.0f, 0.0f });
    mBoxFace[2]->localTransform.SetTranslation(translate);
    mBoxFace[2]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[2]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = green;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[2]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[2]);
    mBox->AttachChild(mBoxFace[2]);

    // -y face
    Vector4<float> darkGreen{ 0.0f, 0.5f, 0.0f, 1.0f };
    mBoxFace[3] = mf.CreateRectangle(2, 2, xExtent, zExtent);
    translate = { 0.0f, -yExtent, 0.0f };
    rotate.SetCol(0, { 1.0f, 0.0f, 0.0f });
    rotate.SetCol(1, { 0.0f, 0.0f, 1.0f });
    rotate.SetCol(2, { 0.0f, -1.0f, 0.0f });
    mBoxFace[3]->localTransform.SetTranslation(translate);
    mBoxFace[3]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[3]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = darkGreen;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[3]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[3]);
    mBox->AttachChild(mBoxFace[3]);

    // +x face
    Vector4<float> blue{ 0.0f, 0.0f, 1.0f, 1.0f };
    mBoxFace[4] = mf.CreateRectangle(2, 2, yExtent, zExtent);
    translate = { xExtent, 0.0f, 0.0f };
    rotate.SetCol(0, { 0.0f, 1.0f, 0.0f });
    rotate.SetCol(1, { 0.0f, 0.0f, 1.0f });
    rotate.SetCol(2, { 1.0f, 0.0f, 0.0f });
    mBoxFace[4]->localTransform.SetTranslation(translate);
    mBoxFace[4]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[4]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = blue;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[4]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[4]);
    mBox->AttachChild(mBoxFace[4]);

    // -x face
    Vector4<float> darkBlue{ 0.0f, 0.0f, 0.5f, 1.0f };
    mBoxFace[5] = mf.CreateRectangle(2, 2, zExtent, yExtent);
    translate = { -xExtent, 0.0f, 0.0f };
    rotate.SetCol(0, { 0.0f, 0.0f, 1.0f });
    rotate.SetCol(1, { 0.0f, 1.0f, 0.0f });
    rotate.SetCol(2, { -1.0f, 0.0f, 0.0f });
    mBoxFace[5]->localTransform.SetTranslation(translate);
    mBoxFace[5]->localTransform.SetRotation(rotate);
    vbuffer = mBoxFace[5]->GetVertexBuffer();
    vertices = vbuffer->Get<VertexPC>();
    for (size_t i = 0; i < 4; ++i)
    {
        vertices[i].color = darkBlue;
    }
    effect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mBoxFace[5]->SetEffect(effect);
    mPVWMatrices.Subscribe(mBoxFace[5]);
    mBox->AttachChild(mBoxFace[5]);

    MoveBox();
}

void RoughPlaneSolidBoxWindow3::InitializeModule()
{
    // Set up the physics module.
    mModule.mu = 0.01;
    mModule.gravity = 10.0;
    mModule.angle = 0.125 * GTE_C_PI;
    mModule.xLocExt = 0.8;
    mModule.yLocExt = 0.4;
    mModule.zLocExt = 0.2;

    // Initialize the differential equations.
    double time = 0.0;
    double deltaTime = 0.0005;
    double x = -6.0;
    double w = 1.0;
    double xDot = 4.0;
    double wDot = 6.0;
    double theta = 0.25 * GTE_C_PI;
    double thetaDot = 4.0;
    mModule.Initialize(time, deltaTime, x, w, theta, xDot, wDot, thetaDot);

    mDoUpdate = true;
}

void RoughPlaneSolidBoxWindow3::MoveBox()
{
    float x = static_cast<float>(mModule.GetX());
    float w = static_cast<float>(mModule.GetW());
    float xExt = static_cast<float>(mModule.xLocExt);
    float yExt = static_cast<float>(mModule.yLocExt);
    float zExt = static_cast<float>(mModule.zLocExt);
    float sinPhi = static_cast<float>(mModule.sinAngle);
    float cosPhi = static_cast<float>(mModule.cosAngle);
    float theta = static_cast<float>(mModule.GetTheta());
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);

    // Compute the box center.
    Vector3<float> center{ x, w * cosPhi - zExt * sinPhi, w * sinPhi + zExt * cosPhi };

    // Compute the box orientation.
    Vector3<float> axis0{ cosTheta, -sinTheta * cosPhi, -sinTheta * sinPhi };
    Vector3<float> axis1{ sinTheta, +cosTheta * cosPhi, +cosTheta * sinPhi };
    Vector3<float> axis2{ 0.0f, -sinPhi, cosPhi };

    // Keep the box from sliding below the ground.
    float zRadius =
        xExt * std::fabs(axis0[2]) +
        yExt * std::fabs(axis1[2]) +
        zExt * std::fabs(axis2[2]);

    if (center[2] >= zRadius)
    {
        // Update the box.
        mBox->localTransform.SetTranslation(center);
        Matrix3x3<float> rotate{};
        rotate.SetCol(0, axis0);
        rotate.SetCol(1, axis1);
        rotate.SetCol(2, axis2);
        mBox->localTransform.SetRotation(rotate);
    }
    else
    {
        mDoUpdate = false;
    }

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void RoughPlaneSolidBoxWindow3::PhysicsTick()
{
    if (mDoUpdate)
    {
        // Execute the physics system at the desired frames per second.
        mCurrPhysicsTime = mPhysicsTimer.GetSeconds();
        double deltaTime = mCurrPhysicsTime - mLastPhysicsTime;
        if (deltaTime >= mModule.GetDeltaTime())
        {
            mModule.Update();
            MoveBox();
            mLastPhysicsTime = mCurrPhysicsTime;
        }
    }
}

void RoughPlaneSolidBoxWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mGround);
    mEngine->Draw(mRamp);
    for (auto const& face : mBoxFace)
    {
        mEngine->Draw(face);
    }
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
}
