// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.12

#include "WaterDropFormationWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>

//#define SINGLE_STEP

WaterDropFormationWindow3::WaterDropFormationWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mWireState{},
    mBlendState{},
    mVFormat{},
    mScene{},
    mCeiling{},
    mWall{},
    mWaterSurface{},
    mWaterDrop{},
    mWaterTexture{},
    mWaterSurfaceRevolution{},
    mWaterDropRevolution{},
    mSpline{},
    mCircle{},
    mTargets{},
    mSimTime(0.0f),
    mSimDelta(0.05f),
    mLastMotionTime(0.0),
    mCurrMotionTime(0.0)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mEngine->SetClearColor({ 0.4f, 0.5f, 0.6f, 1.0f });

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::Mode::INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::Mode::SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::Mode::INV_SRC_ALPHA;

    float angle = static_cast<float>(0.01 * GTE_C_PI);
    float cs = cos(angle), sn = sin(angle);
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.01f, 0.001f,
        { 21.1804028f, 0.0f, 0.665620983f }, { -cs, 0.0f, -sn }, { sn, 0.0f, -cs });

    CreateScene();
}

void WaterDropFormationWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

#if !defined(SINGLE_STEP)
    // Execute the physics system at the desired frames per second.
    mCurrMotionTime = mMotionTimer.GetSeconds();
    double deltaTime = mCurrMotionTime - mLastMotionTime;
    if (deltaTime >= 1.0 / 30.0)
    {
        PhysicsTick();
        mLastMotionTime = mCurrMotionTime;
    }
#endif
    GraphicsTick();

    mTimer.UpdateFrameCount();
}

bool WaterDropFormationWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

#if defined(SINGLE_STEP)
    case 'g':
    case 'G':
        PhysicsTick();
        return true;
#endif
    }

    return Window3::OnCharPress(key, x, y);
}

bool WaterDropFormationWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    std::vector<std::string> inputs =
    {
        "StoneWall.png",
        "Water.png"
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

void WaterDropFormationWindow3::CreateScene()
{
    // Vertex format shared by the ceiling, wall, and water surfaces.
    mVFormat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mVFormat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);

    mScene = std::make_shared<Node>();

    CreateCeilingAndWall();
    CreateWaterRoot();
    CreateConfiguration0();

    mScene->localTransform.SetTranslation(4.0f, 0.0f, -4.0f);
    mTrackBall.Attach(mScene);
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void WaterDropFormationWindow3::CreateCeilingAndWall()
{
    MeshFactory mf;
    mf.SetVertexFormat(mVFormat);

    auto texture = WICFileIO::Load(mEnvironment.GetPath("StoneWall.png"), true);
    texture->AutogenerateMipmaps();

    mCeiling = mf.CreateRectangle(2, 2, 8.0f, 16.0f);
    std::shared_ptr<VertexBuffer> vbuffer = mCeiling->GetVertexBuffer();
    uint32_t numVertices = vbuffer->GetNumElements();
    Vertex* vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        vertices[i].tcoord[1] *= 2.0f;
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
    mCeiling->SetEffect(effect);
    mPVWMatrices.Subscribe(mCeiling->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mCeiling);

    mWall = mf.CreateRectangle(2, 2, 16.0f, 8.0f);
    vbuffer = mWall->GetVertexBuffer();
    numVertices = vbuffer->GetNumElements();
    vertices = vbuffer->Get<Vertex>();
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vector3<float> pos = vertices[i].position;
        vertices[i].position = { pos[2] - 8.0f, pos[0], pos[1] + 8.0f };
        vertices[i].tcoord[0] *= 2.0f;
    }

    effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::WRAP, SamplerState::WRAP);
    mWall->SetEffect(effect);
    mPVWMatrices.Subscribe(mWall->worldTransform, effect->GetPVWMatrixConstant());
    mScene->AttachChild(mWall);
}

void WaterDropFormationWindow3::CreateWaterRoot()
{
    // The texture shared by the water drop surfaces. Modify the alpha
    // channel for transparency.
    mWaterTexture = WICFileIO::Load(mEnvironment.GetPath("Water.png"), true);
    mWaterTexture->AutogenerateMipmaps();
    uint32_t numTexels = mWaterTexture->GetNumElements();
    uint32_t* texels = mWaterTexture->Get<uint32_t>();
    for (uint32_t i = 0; i < numTexels; ++i)
    {
        texels[i] = (texels[i] & 0x00FFFFFF) | 0x80000000;
    }

    mWaterRoot = std::make_shared<Node>();
    mWaterRoot->localTransform.SetTranslation(0.0f, 0.0f, 0.1f);
    mWaterRoot->localTransform.SetUniformScale(8.0f);
    mScene->AttachChild(mWaterRoot);
}

void WaterDropFormationWindow3::CreateSpline0AndTargets()
{
    int32_t constexpr numControls = 13;
    int32_t constexpr degree = 2;
    BasisFunctionInput<float> input(numControls, degree);
    mSpline = std::make_shared<NURBSCurve<2, float>>(input, nullptr, nullptr);

    for (int32_t i = 0; i < mSpline->GetNumControls(); ++i)
    {
        mSpline->SetControl(i, { 0.125f + 0.0625f * i, 0.0625f });
        mSpline->SetWeight(i, 1.0f);
    }

    float constexpr modWeight = 0.3f;
    mSpline->SetWeight(3, modWeight);
    mSpline->SetWeight(5, modWeight);
    mSpline->SetWeight(7, modWeight);
    mSpline->SetWeight(9, modWeight);

    float constexpr h = 0.5f;
    float constexpr d = 0.0625f;
    float constexpr extra = 0.1f;
    mTargets.resize(numControls);
    mTargets[0] = mSpline->GetControl(0);
    mTargets[1] = mSpline->GetControl(6);
    mTargets[2] = { mSpline->GetControl(6)[0], h - d - extra };
    mTargets[3] = { mSpline->GetControl(5)[0], h - d - extra };
    mTargets[4] = { mSpline->GetControl(5)[0], h };
    mTargets[5] = { mSpline->GetControl(5)[0], h + d };
    mTargets[6] = { mSpline->GetControl(6)[0], h + d };
    mTargets[7] = { mSpline->GetControl(7)[0], h + d };
    mTargets[8] = { mSpline->GetControl(7)[0], h };
    mTargets[9] = { mSpline->GetControl(7)[0], h - d - extra };
    mTargets[10] = { mSpline->GetControl(6)[0], h - d - extra };
    mTargets[11] = mSpline->GetControl(6);
    mTargets[12] = mSpline->GetControl(12);

    // Restrict evaluation to a subinterval of the domain.
    mSpline->SetTimeInterval(0.5f, 1.0f);
}

void WaterDropFormationWindow3::CreateConfiguration0()
{
    mWaterRoot->DetachChildAt(0);
    mWaterRoot->DetachChildAt(1);
    mCircle = nullptr;
    mSimTime = 0.0f;

    CreateSpline0AndTargets();

    // Create the water surface.
    mPVWMatrices.Unsubscribe(mWaterSurface);
    mWaterSurfaceRevolution = std::make_shared<RevolutionSurface>(
        mSpline, mSpline->GetControl(6)[0], RevolutionSurface::REV_DISK_TOPOLOGY,
        32, 16, mVFormat, false, true, true);
    mWaterSurface = mWaterSurfaceRevolution->GetSurface();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, mWaterTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mWaterSurface->SetEffect(effect);
    mPVWMatrices.Subscribe(mWaterSurface);
    mWaterRoot->AttachChild(mWaterSurface);
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void WaterDropFormationWindow3::CreateSpline1()
{
    // Create the spline used to generate the water surface of revolution.
    // This surface is used after the water drop separates from the initial
    // surface.
    int32_t constexpr numControls = 5;
    int32_t constexpr degree = 2;
    std::vector<Vector2<float>> controls(numControls);
    std::vector<float> weights(numControls);

    controls[0] = mSpline->GetControl(0);
    controls[1] = mSpline->GetControl(1);
    controls[2] = 0.5f * (mSpline->GetControl(1) + mSpline->GetControl(2));
    controls[3] = mSpline->GetControl(11);
    controls[4] = mSpline->GetControl(12);

    weights[0] = 1.0f;
    weights[1] = 1.0f;
    weights[2] = 1.0f;
    weights[3] = 1.0f;
    weights[4] = 1.0f;

    BasisFunctionInput<float> input(numControls, degree);
    mSpline = std::make_shared<NURBSCurve<2, float>>(
        input, controls.data(), weights.data());

    // Restrict the evaluation to a subdomain of the spline for the initial
    // water surface.
    mSpline->SetTimeInterval(0.5f, 1.0f);
}

void WaterDropFormationWindow3::CreateCircle1()
{
    // Create the circle used to generate the water drop of revolution.
    // Replicate the first two control-weights to obtain C1 continuity for the
    // periodic curve. The circle NURBS is a loop and not open. The curve is
    // constructed with degree-2 replicated control points. Although the curve
    // is geometrically symmetric about the vertical axis, it is not symmetric
    // in t about the half way point (0.5) of the domain [0,1].
    int32_t constexpr numControls = 11;
    int32_t constexpr degree = 2;
    std::vector<Vector2<float>> controls(numControls);
    std::vector<float> weights(numControls);

    controls[0] = 0.25f * mSpline->GetControl(1) + 0.75f * mSpline->GetControl(2);
    controls[1] = mSpline->GetControl(3);
    controls[2] = mSpline->GetControl(4);
    controls[3] = mSpline->GetControl(5);
    controls[4] = mSpline->GetControl(6);
    controls[5] = mSpline->GetControl(7);
    controls[6] = mSpline->GetControl(8);
    controls[7] = mSpline->GetControl(9);
    controls[8] = 0.25f * mSpline->GetControl(1) + 0.75f * mSpline->GetControl(2);
    controls[9] = controls[0];
    controls[10] = controls[1];

    weights[0] = 1.0f;
    weights[1] = mSpline->GetWeight(3);
    weights[2] = 1.0f;
    weights[3] = mSpline->GetWeight(5);
    weights[4] = 1.0f;
    weights[5] = mSpline->GetWeight(7);
    weights[6] = 1.0f;
    weights[7] = mSpline->GetWeight(9);
    weights[8] = 1.0f;
    weights[9] = weights[0];
    weights[10] = weights[1];

    BasisFunctionInput<float> input{};
    input.numControls = numControls;
    input.degree = degree;
    input.uniform = true;
    input.periodic = true;
    input.numUniqueKnots = input.numControls + input.degree + 1;
    input.uniqueKnots.resize(input.numUniqueKnots);
    float const fNumControls = static_cast<float>(numControls);
    float const fDegree = static_cast<float>(degree);
    float denom = fNumControls - fDegree;
    for (int32_t i = 0; i < input.numUniqueKnots; ++i)
    {
        input.uniqueKnots[i].t = (static_cast<float>(i) - fDegree) / denom;
        input.uniqueKnots[i].multiplicity = 1;
    }
    mCircle = std::make_shared<NURBSCurve<2, float>>(
        input, controls.data(), weights.data());

    // Restrict evaluation to a subinterval of the domain.
    mCircle->SetTimeInterval(0.375f, 1.0f);
}

void WaterDropFormationWindow3::CreateConfiguration1()
{
    CreateCircle1();
    CreateSpline1();

    // Switch to the water surface that is detached from the water drop.
    mPVWMatrices.Unsubscribe(mWaterSurface);
    mWaterRoot->DetachChild(mWaterSurface);

    mWaterSurfaceRevolution->SetCurve(mSpline);
    mWaterSurface = mWaterSurfaceRevolution->GetSurface();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, mWaterTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mWaterSurface->SetEffect(effect);

    mWaterRoot->AttachChild(mWaterSurface);
    mPVWMatrices.Subscribe(mWaterSurface);

    // Create water drop. The outside view value is set to 'false' because
    // the curve (x(t),z(t)) has the property dz/dt < 0. If the curve
    // instead had the property dz/dt > 0, then 'true' is the correct value
    // for the outside view.
    mPVWMatrices.Unsubscribe(mWaterDrop);
    mWaterDropRevolution = std::make_shared<RevolutionSurface>(
        mCircle,
        mCircle->GetControl(9)[0],
        RevolutionSurface::REV_DISK_TOPOLOGY,
        32, 16, mVFormat, false, false, true);
    mWaterDrop = mWaterDropRevolution->GetSurface();

    effect = std::make_shared<Texture2Effect>(mProgramFactory, mWaterTexture,
        SamplerState::MIN_L_MAG_L_MIP_L, SamplerState::CLAMP, SamplerState::CLAMP);
    mWaterDrop->SetEffect(effect);

    mWaterRoot->AttachChild(mWaterDrop);
    mPVWMatrices.Subscribe(mWaterDrop);

    mTrackBall.Update();
    mPVWMatrices.Update();
}

void WaterDropFormationWindow3::DoPhysical1()
{
    // Modify control points.
    float t = mSimTime, oneMinusT = 1.0f - t;
    float t2 = t * t, oneMinusT2 = 1.0f - t2;
    int32_t numControls = mSpline->GetNumControls();
    for (int32_t i = 0; i < numControls; ++i)
    {
        if (i != 4)
        {
            mSpline->SetControl(i, oneMinusT * mSpline->GetControl(i) + t * mTargets[i]);
        }
        else
        {
            mSpline->SetControl(i, oneMinusT2 * mSpline->GetControl(i) + t2 * mTargets[i]);
        }
    }

    // Modify mesh vertices.
    mWaterSurfaceRevolution->UpdateSurface();
    mEngine->Update(mWaterSurface->GetVertexBuffer());
}

void WaterDropFormationWindow3::DoPhysical2()
{
    if (!mCircle)
    {
        CreateConfiguration1();
    }

    mSimTime += mSimDelta;

    // Surface evolves to a disk.
    float t = mSimTime - 1.0f, oneMinusT = 1.0f - t;
    Vector2<float> control = oneMinusT * mSpline->GetControl(2) + t * mSpline->GetControl(1);
    mSpline->SetControl(2, control);

    // Sphere floats down a little bit.
    int32_t const numControls = mCircle->GetNumControls();
    for (int32_t i = 0; i < numControls; ++i)
    {
        control = mCircle->GetControl(i) + Vector2<float>{ 0.0f, 1.0f / 32.0f};
        mCircle->SetControl(i, control);
    }

    mWaterSurfaceRevolution->UpdateSurface();
    mEngine->Update(mWaterSurface->GetVertexBuffer());
    mWaterDropRevolution->UpdateSurface();
    mEngine->Update(mWaterDrop->GetVertexBuffer());
}

void WaterDropFormationWindow3::DoPhysical3()
{
    mSimTime += mSimDelta;

    // Sphere floats down a little bit.
    int32_t const numControls = mCircle->GetNumControls();
    for (int32_t i = 0; i < numControls; ++i)
    {
        Vector2<float> control = mCircle->GetControl(i);
        if (i == 0 || i == numControls - 1)
        {
            control[1] += 1.3f / 32.0f;
        }
        else
        {
            control[1] += 1.0f / 32.0f;
        }
        mCircle->SetControl(i, control);
    }

    mWaterDropRevolution->UpdateSurface();
    mEngine->Update(mWaterDrop->GetVertexBuffer());
}

void WaterDropFormationWindow3::PhysicsTick()
{
    mSimTime += mSimDelta;
    if (mSimTime <= 1.0f)
    {
        // Water surface extruded to form a water drop.
        DoPhysical1();
    }
    else if (mSimTime <= 2.0f)
    {
        // Water drop splits from water surface.
        DoPhysical2();
    }
    else if (mSimTime <= 4.0f)
    {
        // Water drop continues downward motion, surface no longer changes.
        DoPhysical3();
    }
    else
    {
        // Restart the animation.
        CreateConfiguration0();
    }
}

void WaterDropFormationWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();

    mEngine->Draw(mCeiling);
    mEngine->Draw(mWall);

    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mWaterSurface);
    if (mCircle)
    {
        mEngine->Draw(mWaterDrop);
    }
    mEngine->SetDefaultBlendState();

    std::array<float, 4> textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    std::string message = "time = " + std::to_string(mSimTime);
    mEngine->Draw(96, mYSize - 8, textColor, message);
    mEngine->Draw(8, mYSize - 8, textColor, mTimer.GetFPS());

    mEngine->DisplayColorBuffer(0);
}
