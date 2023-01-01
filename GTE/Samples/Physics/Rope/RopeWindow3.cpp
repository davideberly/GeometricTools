// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "RopeWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/Texture2Effect.h>

RopeWindow3::RopeWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fill = RasterizerState::Fill::WIREFRAME;
    mEngine->SetClearColor({ 0.75f, 0.85f, 0.95f, 1.0f });

    CreateSprings();
    CreateRope();
    InitializeCamera(60.0f, GetAspectRatio(), 0.1f, 1000.0f, 0.001f, 0.001f,
        { 0.0f, 1.25f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();

    mTime0 = std::chrono::high_resolution_clock::now();
}

void RopeWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    // Clamp to 120 frames per second.
    mTime1 = std::chrono::high_resolution_clock::now();
    int64_t delta = std::chrono::duration_cast<std::chrono::milliseconds>(
        mTime1 - mTime0).count();

    if (120 * delta >= 1000)
    {
        mTime0 = mTime1;
        PhysicsTick();
        GraphicsTick();
    }

    mTimer.UpdateFrameCount();
}

bool RopeWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
{
    switch (key)
    {
    case 'w':  // toggle wireframe
    case 'W':
        if (mWireState != mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mWireState);
        }
        else
        {
            mEngine->SetDefaultRasterizerState();
        }
        return true;
    case 'm':  // decrease mass
        if (mModule->GetMass(1) > 0.05f)
        {
            for (int32_t i = 1; i < mModule->GetNumParticles() - 1; ++i)
            {
                mModule->SetMass(i, mModule->GetMass(i) - 0.01f);
            }
        }
        return true;
    case 'M':  // increase mass
        for (int32_t i = 1; i < mModule->GetNumParticles() - 1; ++i)
        {
            mModule->SetMass(i, mModule->GetMass(i) + 0.01f);
        }
        return true;
    case 'c':  // decrease spring constant
        if (mModule->GetConstant(0) > 0.05f)
        {
            for (int32_t i = 0; i < mModule->GetNumSprings(); ++i)
            {
                mModule->SetConstant(i, mModule->GetConstant(i) - 0.01f);
            }
        }
        return true;
    case 'C':  // increase spring constant
        for (int32_t i = 0; i < mModule->GetNumSprings(); ++i)
        {
            mModule->SetConstant(i, mModule->GetConstant(i) + 0.01f);
        }
        return true;
    case 'l':  // decrease spring resting length
        if (mModule->GetLength(0) > 0.05f)
        {
            for (int32_t i = 0; i < mModule->GetNumSprings(); ++i)
            {
                mModule->SetLength(i, mModule->GetLength(i) - 0.01f);
            }
        }
        return true;
    case 'L':  // increase spring resting length
        for (int32_t i = 0; i < mModule->GetNumSprings(); ++i)
        {
            mModule->SetLength(i, mModule->GetLength(i) + 0.01f);
        }
        return true;
    case 'f':  // toggle wind force on/off
    case 'F':
        mModule->enableWind = !mModule->enableWind;
        return true;
    case 'r':  // toggle random wind direction change on/off
    case 'R':
        mModule->enableWindChange = !mModule->enableWindChange;
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool RopeWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("Rope.png") == "")
    {
        LogError("Cannot find file Rope.png.");
        return false;
    }

    return true;
}

void RopeWindow3::CreateSprings()
{
    int32_t numParticles = 8;
    float step = 0.1f;
    Vector3<float> gravity{ 0.0f, 0.0f, -1.0f };
    Vector3<float> wind{ 0.0f, -0.25f, 0.0f };
    float windChangeAmplitude = 0.01f;
    float viscosity = 10.0f;
    mModule = std::make_unique<PhysicsModule>(numParticles, step, gravity,
        wind, windChangeAmplitude, viscosity);

    // Constant mass at interior points (endpoints are immovable).
    mModule->SetMass(0, std::numeric_limits<float>::max());
    mModule->SetMass(numParticles - 1, std::numeric_limits<float>::max());
    for (int32_t i = 1; i < numParticles - 1; ++i)
    {
        mModule->SetMass(i, 1.0f);
    }

    // Initial position on a horizontal line segment.
    float factor = 1.0f / static_cast<float>(numParticles - 1);
    for (int32_t i = 0; i < numParticles; ++i)
    {
        mModule->SetPosition(i, { i * factor, 0.0f, 1.0f });
    }

    // Initial velocities are all zero.
    for (int32_t i = 0; i < numParticles; ++i)
    {
        mModule->SetVelocity(i, Vector3<float>::Zero());
    }

    // Springs are at rest in the initial horizontal configuration.
    int32_t numSprings = numParticles - 1;
    float restLength = 1.0f / static_cast<float>(numSprings);
    for (int32_t i = 0; i < numSprings; ++i)
    {
        mModule->SetConstant(i, 10.0f);
        mModule->SetLength(i, restLength);
    }
}

void RopeWindow3::CreateRope()
{
    MeshDescription desc(MeshTopology::CYLINDER, 64, 8);

    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    auto vbuffer = std::make_shared<VertexBuffer>(vformat, desc.numVertices);
    vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
    auto vertices = vbuffer->Get<Vertex>();
    auto ibuffer = std::make_shared<IndexBuffer>(IP_TRIMESH, desc.numTriangles, sizeof(uint32_t));

    desc.vertexAttributes =
    {
        VertexAttribute("position", &vertices[0].position, sizeof(Vertex)),
        VertexAttribute("tcoord", &vertices[0].tcoord, sizeof(Vertex))
    };

    desc.indexAttribute = IndexAttribute(ibuffer->GetData(), ibuffer->GetElementSize());

    BasisFunctionInput<float> input(mModule->GetNumParticles(), 2);
    mSpline = std::make_shared<BSplineCurve<3, float>>(input, &mModule->GetPosition(0));
    mSurface = std::make_unique<TubeMesh<float>>(desc, mSpline,
        [](float){ return 0.025f; }, false, false, Vector3<float>{ 0.0f, 0.0f, 1.0f });

    std::string path = mEnvironment.GetPath("Rope.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);

    mRope = std::make_shared<Visual>(vbuffer, ibuffer, effect);
    mRope->UpdateModelBound();
    mRope->localTransform.SetTranslation(-mRope->modelBound.GetCenter());
    mPVWMatrices.Subscribe(mRope->worldTransform, effect->GetPVWMatrixConstant());

    mTrackBall.Attach(mRope);
    mTrackBall.Update();
}

void RopeWindow3::PhysicsTick()
{
    // Forces are independent of time, just pass in t = 0.
    mModule->Update(0.0f);

    // Update spline curve.  Remember that the spline maintains its own copy
    // of the control points, so this update is necessary.
    int32_t numControls = mModule->GetNumParticles();
    for (int32_t i = 0; i < numControls; ++i)
    {
        mSpline->SetControl(i, mModule->GetPosition(i));
    }

    // Update the GPU copy of the vertices.
    mSurface->Update();
    mEngine->Update(mRope->GetVertexBuffer());
    mPVWMatrices.Update();
}

void RopeWindow3::GraphicsTick()
{
    mEngine->ClearBuffers();
    mEngine->Draw(mRope);
    mEngine->DisplayColorBuffer(0);
}
