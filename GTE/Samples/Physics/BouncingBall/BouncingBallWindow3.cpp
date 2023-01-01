// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.03.07

#include "BouncingBallWindow3.h"
#include <Applications/WICFileIO.h>

BouncingBallWindow3::BouncingBallWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mSimTime(0.0f),
#if defined(BOUNCING_BALL_SINGLE_STEP)
    mSimDelta(0.05f)
#else
    mSimDelta(0.0005f)
#endif
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.5f, 0.0f, 1.0f, 1.0f });
    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    CreateScene();

    // Initial update of objects.
    mScene->Update();
    mBallNode->Update();

    // Initialize ball with correct transformations.
    PhysicsTick();

    float angle = static_cast<float>(0.1 * GTE_C_PI);
    float cs = std::cos(angle), sn = std::sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.1f, 0.01f,
        { 6.75f, 0.0f, 2.3f }, { -cs, 0.0f, -sn }, { -sn, 0.0f, cs });
    mPVWMatrices.Update();
}

void BouncingBallWindow3::OnIdle()
{
#if !defined(BOUNCING_BALL_SINGLE_STEP)
    PhysicsTick();
#endif
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }
    GraphicsTick();
}

bool BouncingBallWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case 's':  // toggle scaling
        mBall->DoAffine() = !mBall->DoAffine();
        return true;

#if defined(BOUNCING_BALL_SINGLE_STEP)
    case 'g':
        mSimTime += mSimDelta;
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool BouncingBallWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "BallTexture.png",
        "Floor.png",
        "Wall1.png"
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

void BouncingBallWindow3::CreateScene()
{
    mScene = std::make_shared<Node>();

    CreateBall();
    CreateFloor();
    CreateWall();
    mScene->AttachChild(mFloor);
    mScene->AttachChild(mWall);

    std::vector<std::shared_ptr<Visual>> planes = { mFloor };
    std::vector<float> reflectances = { 0.2f };
    mPlanarReflectionEffect = std::make_unique<PlanarReflectionEffect>(
        mBallNode, planes, reflectances);
}

void BouncingBallWindow3::CreateBall()
{
    std::string path = mEnvironment.GetPath("BallTexture.png");
    auto texture = WICFileIO::Load(path, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mBall = std::make_unique<DeformableBall>(1.0f, 2.0f, effect);
    mBallNode = std::make_shared<Node>();
    auto const& mesh = mBall->GetMesh();
    mBallNode->AttachChild(mesh);
    mPVWMatrices.Subscribe(mesh->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow3::CreateFloor()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto vertices = vbuffer->Get<Vertex>();
    float const xExtent = 8.0f, yExtent = 16.0f, zValue = 0.0f;
    vertices[0].position = { -xExtent, -yExtent, zValue };
    vertices[1].position = { +xExtent, -yExtent, zValue };
    vertices[2].position = { +xExtent, +yExtent, zValue };
    vertices[3].position = { -xExtent, +yExtent, zValue };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].tcoord = { 1.0f, 0.0f };
    vertices[2].tcoord = { 1.0f, 1.0f };
    vertices[3].tcoord = { 0.0f, 1.0f };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Floor.png");
    auto texture = WICFileIO::Load(path, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mFloor = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mFloor->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow3::CreateWall()
{
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    auto vbuffer = std::make_shared<VertexBuffer>(vformat, 4);
    auto vertices = vbuffer->Get<Vertex>();
    float const xValue = -8.0f, yExtent = 16.0f, zExtent = 16.0f, maxTCoord = 4.0f;

    vertices[0].position = { xValue, -yExtent, 0.0f };
    vertices[1].position = { xValue, +yExtent, 0.0f };
    vertices[2].position = { xValue, +yExtent, zExtent };
    vertices[3].position = { xValue, -yExtent, zExtent };
    vertices[0].tcoord = { 0.0f, 0.0f };
    vertices[1].tcoord = { maxTCoord, 0.0f };
    vertices[2].tcoord = { maxTCoord, maxTCoord };
    vertices[3].tcoord = { 0.0f, maxTCoord };

    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, 2, sizeof(uint32_t));
    auto indices = ibuffer->Get<uint32_t>();
    indices[0] = 0;  indices[1] = 1;  indices[2] = 2;
    indices[3] = 0;  indices[4] = 2;  indices[5] = 3;

    std::string path = mEnvironment.GetPath("Wall1.png");
    auto texture = WICFileIO::Load(path, false);
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_P, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mWall = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mPVWMatrices.Subscribe(mWall->worldTransform, effect->GetPVWMatrixConstant());
}

void BouncingBallWindow3::PhysicsTick()
{
    // Update the ball.
    mBall->DoSimulationStep(mSimTime);
    mEngine->Update(mBall->GetMesh()->GetVertexBuffer());

    // Get the ball parameters.
    float period = mBall->GetPeriod();
    float tMin = mBall->GetMinActive();
    float tMax = mBall->GetMaxActive();

    // Translate the ball.
    float const yMax = 2.5f, zMax = 0.75f;
    float yTrn, zTrn, ratio, amp;
    float time = std::fmod(mSimTime, 2.0f * period);
    if (time < tMin)
    {
        ratio = time / tMin;
        yTrn = yMax * ratio;
        zTrn = zMax * (1.0f - ratio*ratio);
    }
    else if (time < tMax)
    {
        yTrn = yMax;
        amp = mBall->GetAmplitude(time);
        if (amp <= 0.999f)
        {
            zTrn = -(1.0f - std::sqrt(1.0f - amp + amp * amp)) / (1.0f - amp);
        }
        else
        {
            zTrn = -0.5f;
        }
    }
    else if (time < period + tMin)
    {
        yTrn = -yMax * (time - period) / tMin;
        zTrn = zMax * (time - tMax) * (period + tMin - time) / (tMin * (period - tMax));
    }
    else if (time < period + tMax)
    {
        yTrn = -yMax;
        amp = mBall->GetAmplitude(time - period);
        if (amp <= 0.999f)
        {
            zTrn = -(1.0f - std::sqrt(1.0f - amp + amp*amp)) / (1.0f - amp);
        }
        else
        {
            zTrn = -0.5f;
        }
    }
    else
    {
        yTrn = yMax * (time - 2.0f * period) / (period - tMax);
        zTrn = zMax * (time - (period + tMax)) * (2.0f * period + tMin - time) / (tMin * (period - tMax));
    }
    mBallNode->localTransform.SetTranslation(0.0f, yTrn, zTrn);

    // Rotate the ball.
    float angle = (1.0f + yTrn) * static_cast<float>(GTE_C_HALF_PI) / yMax;
    mBallNode->localTransform.SetRotation(AxisAngle<4, float>(Vector4<float>::Unit(2), angle));

    // Update the scene graph.
    mBallNode->Update();
    mPVWMatrices.Update();

    // Next simulation time.
    mSimTime += mSimDelta;
}

void BouncingBallWindow3::GraphicsTick()
{
    mTimer.Measure();

    mEngine->ClearBuffers();
    mEngine->Draw(mWall);
    mPlanarReflectionEffect->Draw(mEngine, mPVWMatrices);

    std::array<float, 4> textColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());
    mEngine->Draw(128, mYSize - 8, textColor, "time = " + std::to_string(mSimTime));

    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}
