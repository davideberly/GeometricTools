// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "BSplineSurfaceFitterWindow3.h"
#include <Applications/WICFileIO.h>
#include <Graphics/MeshFactory.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/VertexColorEffect.h>
#include <Mathematics/BSplineSurfaceFit.h>
#include <random>

BSplineSurfaceFitterWindow3::BSplineSurfaceFitterWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    if (!SetEnvironment())
    {
        parameters.created = false;
        return;
    }

    mNoCullState = std::make_shared<RasterizerState>();
    mNoCullState->cullMode = RasterizerState::CULL_NONE;

    mNoCullWireState = std::make_shared<RasterizerState>();
    mNoCullWireState->cullMode = RasterizerState::CULL_NONE;
    mNoCullWireState->fillMode = RasterizerState::FILL_WIREFRAME;

    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mEngine->SetRasterizerState(mNoCullState);
    mEngine->SetClearColor({ 0.0f, 0.5f, 0.75f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 0.01f, 100.0f, 0.005f, 0.002f,
        { 0.0f, -9.0f, 1.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mPVWMatrices.Update();
}

void BSplineSurfaceFitterWindow3::OnIdle()
{
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();
    mEngine->Draw(mHeightField);
    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mFittedField);
    mEngine->SetDefaultBlendState();
    mEngine->DisplayColorBuffer(0);
}

bool BSplineSurfaceFitterWindow3::OnCharPress(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
    case 'W':
        if (mNoCullState == mEngine->GetRasterizerState())
        {
            mEngine->SetRasterizerState(mNoCullWireState);
        }
        else
        {
            mEngine->SetRasterizerState(mNoCullState);
        }
        return true;
    }

    return Window3::OnCharPress(key, x, y);
}

bool BSplineSurfaceFitterWindow3::SetEnvironment()
{
    std::string path = GetGTEPath();
    if (path == "")
    {
        return false;
    }

    mEnvironment.Insert(path + "/Samples/Data/");

    if (mEnvironment.GetPath("BTHeightField.png") == "")
    {
        LogError("Cannot find file BTHeightField.png.");
        return false;
    }

    return true;
}

void BSplineSurfaceFitterWindow3::CreateScene()
{
    // Begin with a flat 64x64 height field.
    int const numSamples = 64;
    float const extent = 8.0f;
    VertexFormat hfformat;
    hfformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    hfformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
    MeshFactory mf;
    mf.SetVertexFormat(hfformat);
    mHeightField = mf.CreateRectangle(numSamples, numSamples, extent, extent);
    int numVertices = numSamples * numSamples;
    auto* hfvertices = mHeightField->GetVertexBuffer()->Get<VertexPT>();

    // Set the heights based on a precomputed height field.  Also create a
    // texture image to go with the height field.
    std::string path = mEnvironment.GetPath("BTHeightField.png");
    auto texture = WICFileIO::Load(path, false);
    auto txeffect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);
    mHeightField->SetEffect(txeffect);

    std::mt19937 mte;
    std::uniform_real_distribution<float> symmr(-0.05f, 0.05f);
    std::uniform_real_distribution<float> intvr(32.0f, 64.0f);
    auto* data = texture->Get<unsigned char>();
    std::vector<Vector3<float>> samplePoints(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        unsigned char value = *data;
        float height = 3.0f * (static_cast<float>(value)) / 255.0f + symmr(mte);
        *data++ = static_cast<unsigned char>(intvr(mte));
        *data++ = 3 * (128 - value / 2) / 4;
        *data++ = 0;
        data++;

        hfvertices[i].position[2] = height;
        samplePoints[i] = hfvertices[i].position;
    }

    // Compute a B-Spline surface with NxN control points, where N < 64.
    // This surface will be sampled to 64x64 and displayed together with the
    // original height field for comparison.
    int const numControls = 32;
    int const degree = 3;
    BSplineSurfaceFit<float> fitter(degree, numControls, numSamples, degree,
        numControls, numSamples, &samplePoints[0]);

    VertexFormat ffformat;
    ffformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    ffformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
    mf.SetVertexFormat(ffformat);
    mFittedField = mf.CreateRectangle(numSamples, numSamples, extent, extent);
    auto* ffvertices = mFittedField->GetVertexBuffer()->Get<VertexPC>();

    Vector4<float> translucent{ 1.0f, 1.0f, 1.0f, 0.5f };
    for (int i = 0; i < numVertices; ++i)
    {
        float u = 0.5f * (ffvertices[i].position[0] / extent + 1.0f);
        float v = 0.5f * (ffvertices[i].position[1] / extent + 1.0f);
        ffvertices[i].position = fitter.GetPosition(u, v);
        ffvertices[i].color = translucent;
    }

    auto vceffect = std::make_shared<VertexColorEffect>(mProgramFactory);
    mFittedField->SetEffect(vceffect);

    mPVWMatrices.Subscribe(mHeightField->worldTransform, txeffect->GetPVWMatrixConstant());
    mPVWMatrices.Subscribe(mFittedField->worldTransform, vceffect->GetPVWMatrixConstant());

    mTrackBall.Attach(mHeightField);
    mTrackBall.Attach(mFittedField);
    mTrackBall.Update();
}
