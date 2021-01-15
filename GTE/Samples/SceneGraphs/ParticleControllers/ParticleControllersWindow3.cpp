// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include "ParticleControllersWindow3.h"
#include <Graphics/Texture2Effect.h>

ParticleControllersWindow3::ParticleControllersWindow3(Parameters& parameters)
    :
    Window3(parameters)
{
    mBlendState = std::make_shared<BlendState>();
    mBlendState->target[0].enable = true;
    mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
    mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
    mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

    mNoDepthState = std::make_shared<DepthStencilState>();
    mNoDepthState->depthEnable = false;
    mEngine->SetDepthStencilState(mNoDepthState);

    mWireState = std::make_shared<RasterizerState>();
    mWireState->fillMode = RasterizerState::FILL_WIREFRAME;
    mEngine->SetDefaultRasterizerState();

    mEngine->SetClearColor({ 0.4f, 0.5f, 0.6f, 1.0f });

    CreateScene();
    InitializeCamera(60.0f, GetAspectRatio(), 1.0f, 1000.0f, 0.001f, 0.001f,
        { 4.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f });
    mTrackBall.Update();
    mPVWMatrices.Update();
}

void ParticleControllersWindow3::OnIdle()
{
    mTimer.Measure();

    mParticles->Update(mApplicationTimer.GetSeconds());
    if (mCameraRig.Move())
    {
        mPVWMatrices.Update();
    }

    mEngine->ClearBuffers();

    mEngine->SetBlendState(mBlendState);
    mEngine->Draw(mParticles);
    mEngine->SetDefaultBlendState();
    mEngine->Draw(8, mYSize - 8, { 0.0f, 0.0f, 0.0f, 1.0f }, mTimer.GetFPS());
    mEngine->DisplayColorBuffer(0);

    mTimer.UpdateFrameCount();
}

bool ParticleControllersWindow3::OnCharPress(unsigned char key, int x, int y)
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

void ParticleControllersWindow3::CreateScene()
{
    // Create the particles.
    VertexFormat vformat;
    vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
    vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);

    std::default_random_engine dre;
    std::uniform_real_distribution<float> urd(-1.0f, 1.0f);
    size_t const numParticles = 32;
    float const sizeAdjust = 1.0f;
    std::vector<Vector4<float>> positionSize(numParticles);
    for (size_t i = 0; i < numParticles; ++i)
    {
        positionSize[i][0] = urd(dre);
        positionSize[i][1] = urd(dre);
        positionSize[i][2] = urd(dre);
        positionSize[i][3] = 0.125f * (urd(dre) + 1.0f);
    }

    mParticles = std::make_shared<Particles>(positionSize, sizeAdjust, vformat);
    mBloodCellController = std::make_shared<BloodCellController>(mCamera, mUpdater);
    mParticles->AttachController(mBloodCellController);

    // Create an image with transparency.
    int const xsize = 32, ysize = 32;
    auto texture = std::make_shared<Texture2>(DF_R8G8B8A8_UNORM, xsize, ysize);
    unsigned int* texels = texture->Get<unsigned int>();

    float factor = 4.0f / static_cast<float>(xsize * xsize + ysize * ysize);
    for (int y = 0, i = 0; y < ysize; ++y)
    {
        for (int x = 0; x < xsize; ++x, ++i)
        {
            // The image is red.
            texels[i] = 0x000000FF;

            // The image is semitransparent within a disk, dropping off to
            // completely transparent outside the disk.
            float dx = 2.0f * static_cast<float>(x) - static_cast<float>(xsize);
            float dy = 2.0f * static_cast<float>(y) - static_cast<float>(ysize);
            float value = factor * (dx * dx + dy * dy);
            if (value < 0.5f)
            {
                value = 255.0f * std::cos(static_cast<float>(GTE_C_PI) * value);
                unsigned int alpha = static_cast<unsigned int>(value);
                texels[i] |= alpha << 24;
            }
        }
    }

    auto effect = std::make_shared<Texture2Effect>(mProgramFactory, texture,
        SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP, SamplerState::CLAMP);
    mParticles->SetEffect(effect);
    mPVWMatrices.Subscribe(mParticles->worldTransform, effect->GetPVWMatrixConstant());
    mTrackBall.Attach(mParticles);
}
