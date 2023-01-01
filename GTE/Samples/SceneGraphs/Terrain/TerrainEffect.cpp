// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TerrainEffect.h"
using namespace gte;

TerrainEffect::TerrainEffect(std::shared_ptr<VisualProgram> const& program,
    std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& detailTexture,
    Vector4<float> const& fogColorDensity)
    :
    mBaseTexture(baseTexture),
    mDetailTexture(detailTexture)
{
    mProgram = program;

    mVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(Matrix4x4<float>), true);

    mFogColorDensityConstant = std::make_shared<ConstantBuffer>(sizeof(Vector4<float>), false);
    *mFogColorDensityConstant->Get<Vector4<float>>() = fogColorDensity;

    mBaseSampler = std::make_shared<SamplerState>();
    mBaseSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    mBaseSampler->mode[0] = SamplerState::Mode::CLAMP;
    mBaseSampler->mode[1] = SamplerState::Mode::CLAMP;

    mDetailSampler = std::make_shared<SamplerState>();
    mDetailSampler->filter = SamplerState::Filter::MIN_L_MAG_L_MIP_L;
    mDetailSampler->mode[0] = SamplerState::Mode::CLAMP;
    mDetailSampler->mode[1] = SamplerState::Mode::CLAMP;

    auto const& vshader = mProgram->GetVertexShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("VWMatrix", mVWMatrixConstant);
    vshader->Set("FogColorDensity", mFogColorDensityConstant);

    auto const& pshader = mProgram->GetPixelShader();
    pshader->Set("baseTexture", mBaseTexture, "baseSampler", mBaseSampler);
    pshader->Set("detailTexture", mDetailTexture, "detailSampler", mDetailSampler);
}

void TerrainEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}
