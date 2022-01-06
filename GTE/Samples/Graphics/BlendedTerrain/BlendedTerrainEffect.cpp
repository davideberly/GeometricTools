// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "BlendedTerrainEffect.h"
#include <Applications/WICFileIO.h>
using namespace gte;

BlendedTerrainEffect::BlendedTerrainEffect(std::shared_ptr<GraphicsEngine> const& engine,
    std::shared_ptr<ProgramFactory> const& factory, Environment const& environment,
    bool& created)
    :
    mFlowDirection(nullptr),
    mPowerFactor(nullptr)
{
    created = false;

    // Load and compile the shaders.
    auto vsPath = environment.GetPath(engine->GetShaderName("BlendedTerrain.vs"));
    auto psPath = environment.GetPath(engine->GetShaderName("BlendedTerrain.ps"));
    mProgram = factory->CreateFromFiles(vsPath, psPath, "");
    if (!mProgram)
    {
        // The program factory will generate Log* messages.
        return;
    }

    // Load the textures.
    auto path = environment.GetPath("BTGrass.png");
    mGrassTexture = WICFileIO::Load(path, true);
    mGrassTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTStone.png");
    mStoneTexture = WICFileIO::Load(path, true);
    mStoneTexture->AutogenerateMipmaps();

    path = environment.GetPath("BTCloud.png");
    mCloudTexture = WICFileIO::Load(path, true);
    mCloudTexture->AutogenerateMipmaps();

    // Create the shader constants.
    mFlowDirectionConstant = std::make_shared<ConstantBuffer>(sizeof(Vector2<float>), true);
    mFlowDirection = mFlowDirectionConstant->Get<Vector2<float>>();
    *mFlowDirection = { 0.0f, 0.0f };

    mPowerFactorConstant = std::make_shared<ConstantBuffer>(sizeof(float), true);
    mPowerFactor = mPowerFactorConstant->Get<float>();
    *mPowerFactor = 1.0f;

    // Create a 1-dimensional texture whose intensities are proportional to
    // height.
    uint32_t const numTexels = 256;
    mBlendTexture = std::make_shared<Texture1>(DF_R8_UNORM, numTexels);
    uint8_t* texels = mBlendTexture->Get<uint8_t>();
    for (uint32_t i = 0; i < numTexels; ++i, ++texels)
    {
        *texels = static_cast<uint8_t>(i);
    }

    // Create the texture samplers.  The common sampler uses trilinear
    // interpolation (mipmapping).  The blend sample uses bilinear
    // interpolation (no mipmapping).
    mCommonSampler = std::make_shared<SamplerState>();
    mCommonSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    mCommonSampler->mode[0] = SamplerState::Mode::WRAP;
    mCommonSampler->mode[1] = SamplerState::Mode::WRAP;
    mBlendSampler = std::make_shared<SamplerState>();
    mBlendSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_P;
    mBlendSampler->mode[0] = SamplerState::Mode::WRAP;

    // Set the resources for the shaders.
    auto const& vshader = mProgram->GetVertexShader();
    auto const& pshader = mProgram->GetPixelShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("FlowDirection", mFlowDirectionConstant);
    pshader->Set("PowerFactor", mPowerFactorConstant);
    pshader->Set("grassTexture", mGrassTexture, "grassSampler", mCommonSampler);
    pshader->Set("stoneTexture", mStoneTexture, "stoneSampler", mCommonSampler);
    pshader->Set("cloudTexture", mCloudTexture, "cloudSampler", mCommonSampler);
    pshader->Set("blendTexture", mBlendTexture, "blendSampler", mBlendSampler);

    created = true;
}

void BlendedTerrainEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
