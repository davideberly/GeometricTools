// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#include "SMUnlitEffect.h"

SMUnlitEffect::SMUnlitEffect(
    std::shared_ptr<ProgramFactory> const& factory,
    std::string const& vsPath,
    std::string const& psPath,
    Geometry const& geometry,
    Screen const& screen,
    std::shared_ptr<Texture2> const& shadowTexture)
    :
    mGeometryBuffer{},
    mScreenBuffer{},
    mShadowTexture(shadowTexture),
    mSampler{}
{
    mProgram = factory->CreateFromFiles(vsPath, psPath, "");
    LogAssert(
        mProgram,
        "Cannot compile " + vsPath + " or " + psPath);

    mGeometryBuffer = std::make_shared<ConstantBuffer>(sizeof(Geometry), true);
    *mGeometryBuffer->Get<Geometry>() = geometry;

    mScreenBuffer = std::make_shared<ConstantBuffer>(sizeof(Screen), true);
    *mScreenBuffer->Get<Screen>() = screen;

    mSampler = std::make_shared<SamplerState>();
    mSampler->filter = SamplerState::Filter::MIN_P_MAG_L_MIP_P;
    mSampler->mode[0] = SamplerState::Mode::CLAMP;
    mSampler->mode[1] = SamplerState::Mode::CLAMP;

    auto const& vshader = mProgram->GetVertexShader();
    vshader->Set("PVWMatrix", mPVWMatrixConstant);
    vshader->Set("Geometry", mGeometryBuffer);

    auto const& pshader = mProgram->GetPixelShader();
    pshader->Set("Screen", mScreenBuffer);
    pshader->Set("shadowTexture", mShadowTexture, "shadowSampler", mSampler);
}
