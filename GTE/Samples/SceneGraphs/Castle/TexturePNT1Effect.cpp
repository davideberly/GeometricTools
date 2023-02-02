// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "TexturePNT1Effect.h"
using namespace gte;

TexturePNT1Effect::TexturePNT1Effect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
    SamplerState::Mode mode0, SamplerState::Mode mode1)
    :
    mTexture(texture)
{
    int32_t api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mSampler = std::make_shared<SamplerState>();
        mSampler->filter = filter;
        mSampler->mode[0] = mode0;
        mSampler->mode[1] = mode1;

        mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetPixelShader()->Set("baseTexture", texture, "baseSampler", mSampler);
    }
}

void TexturePNT1Effect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

// NOTE: The only difference between the GLSL vertex shader of
// TexturePNT1Effect and Texture2Effect is the location of modelTCoord.
std::string const TexturePNT1Effect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 2) in vec2 modelTCoord;
    layout(location = 0) out vec2 vertexTCoord;

    void main()
    {
        vertexTCoord = modelTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const TexturePNT1Effect::msGLSLPSSource =
R"(
    uniform sampler2D baseSampler;

    layout(location = 0) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = texture(baseSampler, vertexTCoord);
    }
)";

std::string const TexturePNT1Effect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float2 vertexTCoord : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    #else
        output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    #endif
        output.vertexTCoord = input.modelTCoord;
        return output;
    }
)";

std::string const TexturePNT1Effect::msHLSLPSSource =
R"(
    Texture2D baseTexture;
    SamplerState baseSampler;

    struct PS_INPUT
    {
        float2 vertexTCoord : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        output.pixelColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
        return output;
    }
)";

ProgramSources TexturePNT1Effect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources TexturePNT1Effect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
