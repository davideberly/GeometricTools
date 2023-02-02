// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "MorphControllersWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/Texture2Effect.h>
#include <random>

MorphControllersWindow3::MorphControllersWindow3(Parameters& parameters)
    :
    Window3(parameters),
    mApplicationTime(0.0),
    mApplicationDeltaTime(0.0001),
    mDrawTargets(false)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cull = RasterizerState::Cull::NONE;
    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->fill = RasterizerState::Fill::WIREFRAME;
    mNoCullWireState->cull = RasterizerState::Cull::NONE;
    mEngine->SetRasterizerState(mNoCullState);

    mEngine->SetClearColor({ 0.75f, 0.75f, 0.75f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -2.35f, 0.075f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
    mTrackBall.Update();
}

void MorphControllersWindow3::OnIdle()
{
    mTimer.Measure();

    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mMorphDisk->Update(mApplicationTime);
    mApplicationTime += mApplicationDeltaTime;

    mEngine->ClearBuffers();

    if (mDrawTargets)
    {
        auto const& saveRState = mEngine->GetRasterizerState();
        mEngine->SetRasterizerState(mNoCullWireState);
        for (auto const& visual : mMorphTarget)
        {
            mEngine->Draw(visual);
        }
        mEngine->SetRasterizerState(saveRState);
    }

    mEngine->Draw(mMorphDisk);

    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool MorphControllersWindow3::OnCharPress(uint8_t key, int32_t x, int32_t y)
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

    case 'd':
    case 'D':
        mDrawTargets = !mDrawTargets;
        return true;
    }
    return Window3::OnCharPress(key, x, y);
}

bool MorphControllersWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");
    if (mEnvironment.GetPath("BlueGrid.png") == "")
    {
        LogError("Cannot find file BlueGrid.png");
        return false;
    }

    return true;
}

void MorphControllersWindow3::CreateScene()
{
    // Start with a disk that is to be morphed.
    VertexFormat vformat;
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VASemantic::TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::DYNAMIC_UPDATE);
    uint32_t const numShellSamples = 16;
    uint32_t const numRadialSamples = 16;
    float const radius = 1.0f;
    mMorphDisk = mf.CreateDisk(numShellSamples, numRadialSamples, radius);
    auto const& diskVBuffer = mMorphDisk->GetVertexBuffer();
    Vertex* diskVertices = diskVBuffer->Get<Vertex>();

    // Create the morph controller.
    size_t numVertices = static_cast<size_t>(diskVBuffer->GetNumElements());
    size_t numTargets = 8;
    size_t numTimes = 129;
    mMorphController = std::make_shared<MorphController>(
        numTargets, numVertices, numTimes, mUpdater);

    mMorphController->repeat = Controller::RepeatType::CYCLE;
    mMorphController->minTime = 0.0;
    mMorphController->maxTime = 1.0;
    mMorphController->phase = 0.0;
    mMorphController->frequency = 1.0;
    mMorphController->active = true;

    // Create the morph controller data.

    // The times are in [0,1].
    std::vector<float> times(numTimes);
    for (size_t key = 0; key < numTimes; ++key)
    {
        times[key] = static_cast<float>(key) / static_cast<float>(numTimes - 1);
    }
    mMorphController->SetTimes(times);

    // Visualize the morph targets.
    mMorphTarget.resize(numTargets);
    vformat.Reset();
    vformat.Bind(VASemantic::POSITION, DF_R32G32B32_FLOAT, 0);
    mf.SetVertexFormat(vformat);
    mf.SetVertexBufferUsage(Resource::Usage::IMMUTABLE);

    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(0.7f, 0.9f);
    std::vector<Vector4<float>> color(numTargets);
    for (size_t i = 0; i < numTargets; ++i)
    {
        color[i] = { urd(dre), urd(dre), urd(dre), 1.0f };
    }

    // Target 0 is the original disk.  The remaining targets have points that
    // are on rays, varying in a sinusoidal manner with amplitude and
    // frequency depending on position (x,y) of the original disk.
    std::vector<Vector3<float>> vertices(numVertices);
    for (size_t i = 0; i < numTargets; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(numTargets - 1);
        for (size_t j = 0; j < numVertices; ++j)
        {
            Vector3<float> p = diskVertices[j].position;
            float amplitude = radius * radius - (p[0] * p[0] + p[1] * p[1]);
            float frequency = 2.35f * p[1];
            float s = amplitude * std::sin(frequency * t);
            float sp1 = s + 1.0f;
            vertices[j][0] = sp1 * p[0];
            vertices[j][1] = sp1 * p[1];
            vertices[j][2] = s;
        }
        mMorphController->SetVertices(i, vertices);

        // Visualize the morph target.
        mMorphTarget[i] = mf.CreateDisk(numShellSamples, numRadialSamples, radius);
        std::memcpy(mMorphTarget[i]->GetVertexBuffer()->GetData(), vertices.data(), numVertices * sizeof(Vector3<float>));
        auto effect = std::make_shared<ConstantColorEffect>(mProgramFactory, color[i]);
        mMorphTarget[i]->SetEffect(effect);
        mPVWMatrices.Subscribe(mMorphTarget[i]->worldTransform, effect->GetPVWMatrixConstant());
        mTrackBall.Attach(mMorphTarget[i]);
    }

    // The weights are computed using a Gaussian distribution.
    std::vector<float> weights(numTargets);
    float const ratio = static_cast<float>(numTargets - 1) / static_cast<float>(numTimes - 1);
    float const factor = std::log(1.0f / 2.0f);
    for (size_t key = 0; key < numTimes; ++key)
    {
        float fkey = static_cast<float>(key);
        float sum = 0.0f;
        for (size_t i = 0; i < numTargets; ++i)
        {
            float fi = static_cast<float>(i);
            float diff = fi - fkey * ratio;
            weights[i] = std::exp(factor * diff * diff);
            sum += weights[i];
        }
        for (size_t i = 0; i < numTargets; ++i)
        {
            weights[i] /= sum;
        }
        mMorphController->SetWeights(key, weights);
    }

    mMorphDisk->AttachController(mMorphController);

    std::string path = mEnvironment.GetPath("BlueGrid.png");
    auto texture = WICFileIO::Load(path, true);
    texture->AutogenerateMipmaps();
    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::Filter::MIN_L_MAG_L_MIP_L, SamplerState::Mode::WRAP,
        SamplerState::Mode::WRAP);
    mMorphDisk->SetEffect(effect);

    mPVWMatrices.Subscribe(mMorphDisk->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mMorphDisk);
    mMorphDisk->Update(mApplicationTime);
}
