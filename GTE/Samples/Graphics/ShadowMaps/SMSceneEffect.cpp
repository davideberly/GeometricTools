// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#include "SMSceneEffect.h"

SMSceneEffect::SMSceneEffect(
    std::shared_ptr<ProgramFactory> const& factory,
    std::string const& vsPath,
    std::string const& psPath,
    Geometry const& geometry,
    LightColor const& lightColor,
    std::shared_ptr<Texture2> const& baseTexture,
    std::shared_ptr<Texture2> const& blurTexture,
    std::shared_ptr<Texture2> const& projTexture)
    :
    mGeometryBuffer{},
    mLightColorBuffer{},
    mBaseTexture(baseTexture),
    mBlurTexture(blurTexture),
    mProjTexture(projTexture),
    mSampler{}
{
    mProgram = factory->CreateFromFiles(vsPath, psPath, "");
    LogAssert(
        mProgram,
        "Cannot compile " + vsPath + " or " + psPath);

    mGeometryBuffer = std::make_shared<ConstantBuffer>(sizeof(Geometry), true);
    *mGeometryBuffer->Get<Geometry>() = geometry;

    mLightColorBuffer = std::make_shared<ConstantBuffer>(sizeof(LightColor), true);
    *mLightColorBuffer->Get<LightColor>() = lightColor;

    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = SamplerState::Filter::MIN_P_MAG_L_MIP_P;
    mSampler->mode[0] = SamplerState::Mode::CLAMP;
    mSampler->mode[1] = SamplerState::Mode::CLAMP;

    auto const& vshader = mProgram->GetVertexShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("Geometry", mGeometryBuffer);

    auto const& pshader = mProgram->GetPixelShader();
    pshader->Set("LightColor", mLightColorBuffer);
    pshader->Set("baseTexture", mBaseTexture, "baseSampler", mSampler);
    pshader->Set("blurTexture", mBlurTexture, "blurSampler", mSampler);
    pshader->Set("projTexture", mProjTexture, "projSampler", mSampler);
}
