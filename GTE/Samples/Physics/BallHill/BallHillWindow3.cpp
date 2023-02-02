// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BallHillWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>

BallHillWindow3::BallHillWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.839215f, 0.894117f, 0.972549f, 1.0f });

    CreateScene();

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = std::cos(angle);
    float sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 100.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 2.0f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();

    mLastPhysicsTime = mPhysicsTimer.GetSeconds();
    mCurrPhysicsTime = 0.0;
}

void BallHillWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(BALL_HILL_SINGLE_STEP)
    // Execute the physics system at 60 frames per second.
    mCurrPhysicsTime = mPhysicsTimer.GetSeconds();
    double deltaTime = mCurrPhysicsTime - mLastPhysicsTime;
    if (deltaTime >= 1.0 / 60.0)
    {
        PhysicsTick();
        mLastPhysicsTime = mCurrPhysicsTime;
    }
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool BallHillWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
#if defined(BALL_HILL_SINGLE_STEP)
    if (key == 'g' || key == 'G')
    {
        PhysicsTick();
        return true;
    }
#endif
    return Window3::OnCharPress(key, x, y);
}

bool BallHillWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "Grass.png",
        "Gravel.png",
        "BallTexture.png"
    };

    for (auto const& input : inputs)
    {
        if (mEnvironment.GetPath(input) == "")
        {
            LogError("Cannot find file " + input);
        }
    }

    return true;
}

void BallHillWindow3::InitializeModule()
{
    mModule.gravity = 1.0f;
    mModule.a1 = 2.0f;
    mModule.a2 = 1.0f;
    mModule.a3 = 1.0f;
    mModule.radius = 0.1f;

    float time = 0.0f;
    float deltaTime = 0.01f;
    float y1 = 0.0f;
    float y2 = 0.0f;
    float y1Dot = 0.1f;
    float y2Dot = 0.1f;
    mModule.Initialize(time, deltaTime, y1, y2, y1Dot, y2Dot);
}

void BallHillWindow3::CreateScene()
{
    InitializeModule();

    mVFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    mMeshFactory.SetVertexFormat(mVFormat);

    CreateGround();
    CreateHill();
    CreateBall();
    CreatePath();
    mTrackBall.Update();
}

void BallHillWindow3::CreateGround()
{
    // Create the ground.  Change the texture repeat pattern.
    mGround = mMeshFactory.CreateRectangle(2, 2, 32.0f, 32.0f);
    auto const& vbuffer = mGround->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord *= 8.0f;
    }

    // Create a texture effect for the ground.
    std::string path = mEnvironment.GetPath("Grass.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);
    mGround->SetEffect(effect);

    mPVWMatrices.Subscribe(mGround->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mGround);
}

void BallHillWindow3::CreateHill()
{
    // Create the hill.  Adjust the disk vertices to form an elliptical
    // paraboloid for the hill.  Change the texture repeat pattern.
    mHill = mMeshFactory.CreateDisk(32, 32, 2.0f);
    auto const& vbuffer = mHill->GetVertexBuffer();
    uint32_t const numVertices = vbuffer->GetNumElements();
    auto vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].position[2] = mModule.GetHeight(vertices[i].position[0], vertices[i].position[1]);
        vertices[i].tcoord *= 8.0f;
    }

    // Create a texture effect for the hill.
    std::string path = mEnvironment.GetPath("Gravel.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);
    mHill->SetEffect(effect);

    mPVWMatrices.Subscribe(mHill->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mHill);
}

void BallHillWindow3::CreateBall()
{
    // Create the ball.
    mBall = mMeshFactory.CreateSphere(16, 16, mModule.radius);

    // Move the ball to the top of the hill.
    Vector3<float> trn = mBall->localTransform.GetTranslation();
    trn[2] = mModule.a3 + mModule.radius;
    mBall->localTransform.SetTranslation(trn);
    UpdateBall();

    // Create a texture effect for the ball.
    std::string path = mEnvironment.GetPath("BallTexture.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP, SamplerState::Mode::WRAP);
    mBall->SetEffect(effect);

    mPVWMatrices.Subscribe(mBall->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mBall);
}

void BallHillWindow3::CreatePath()
{
    // Create the vertex buffer for the path.  All points are initially at the
    // origin but will be dynamically updated.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    uint32_t const numVertices = 1024;
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    vbuffer->SetNumActiveElements(0);
    std::memset(vbuffer->GetData(), 0, vbuffer->GetNumBytes());

    // Create a polyline of contiguous line segments.
    auto ibuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_CONTIGUOUS, numVertices - 1);

    // Create a vertex color effect for the path.
    auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory,
        Vector4<float>{ 1.0f, 1.0f, 1.0f, 1.0f });
    mPath = std::make_shared<Visual>(vbuffer, ibuffer, effect);

    mPVWMatrices.Subscribe(mPath->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mPath);
}

Vector4<float> BallHillWindow3::UpdateBall()
{
    // Compute the location of the center of the ball and the incremental
    // rotation implied by its motion.
    Vector4<float> center;
    Matrix4x4<float> incrRot;
    mModule.GetData(center, incrRot);

    // Update the ball position and orientation.
    mBall->localTransform.SetTranslation(center);
    Matrix4x4<float> orient = mBall->localTransform.GetRotation();
    mBall->localTransform.SetRotation(DoTransform(incrRot, orient));

    // Return the new ball center for further use by application.
    return center;
}

void BallHillWindow3::PhysicsTick()
{
    // Allow motion only while ball is above the ground level.
    if (mBall->localTransform.GetTranslation()[2] <= mModule.radius)
    {
        return;
    }

    // Move the ball.
    mModule.Update();
    Vector4<float> center = UpdateBall();
    mTrackBall.Update();
    mPVWMatrices.Update();

    // Draw only the active quantity of path points for the initial portion
    // of the simulation.  Once all points are activated, then all are drawn.
    auto const& vbuffer = mPath->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    uint32_t numActive = vbuffer->GetNumActiveElements();
    if (numActive < numVertices)
    {
        vbuffer->SetNumActiveElements(++numActive);
        auto position = vbuffer->Get<Vector3<float>>();
        position[numActive] = { center[0], center[1], center[2] };
        if (numActive == 1)
        {
            position[0] = position[1];
        }
        mEngine->Update(vbuffer);
    }
}

void BallHillWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mGround);
    mEngine->Draw(mHill);
    mEngine->Draw(mBall);
    mEngine->Draw(mPath);
    mEngine->Draw(8, mYSize - 8, { 1.0f, 1.0f, 1.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);
}
