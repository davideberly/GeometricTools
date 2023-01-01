// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/VolumeFogEffect.h>
using namespace gte;

VolumeFogEffect::VolumeFogEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture,
    SamplerState::Filter filter, SamplerState::Mode mode0,
    SamplerState::Mode mode1)
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
    else
    {
        LogError("Failed to compile shader programs.");
    }
}

void VolumeFogEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}

std::string const VolumeFogEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec4 modelColor;
    layout(location = 2) in vec2 modelTCoord;
    layout(location = 0) out vec4 vertexColor;
    layout(location = 1) out vec2 vertexTCoord;

    void main()
    {
        vertexColor = modelColor;
        vertexTCoord = modelTCoord;
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
    #else
        gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
    #endif
    }
)";

std::string const VolumeFogEffect::msGLSLPSSource =
R"(
    uniform sampler2D baseSampler;

    layout(location = 0) in vec4 vertexColor;
    layout(location = 1) in vec2 vertexTCoord;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        vec4 textureColor = texture(baseSampler, vertexTCoord);
        pixelColor.rgb = (1.0f - vertexColor.a) * textureColor.rgb +
            vertexColor.a * vertexColor.rgb;
        pixelColor.a = 1.0f;
    }
)";

std::string const VolumeFogEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float4 modelColor : COLOR0;
        float2 modelTCoord : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float4 vertexColor : COLOR0;
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
        output.vertexColor = input.modelColor;
        output.vertexTCoord = input.modelTCoord;
        return output;
    }
)";

std::string const VolumeFogEffect::msHLSLPSSource =
R"(
    Texture2D baseTexture;
    SamplerState baseSampler;

    struct PS_INPUT
    {
        float4 vertexColor : COLOR0;
        float2 vertexTCoord : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        // The blending equation is
        //   (rf,gf,bf) = (1-av)*(rt,gt,bt) + av*(rv,gv,bv)
        // where (rf,gf,bf) is the final color, (rt,gt,bt) is the texture color,
        // and (rv,gv,bv,av) is the vertex color.

        PS_OUTPUT output;
        float4 textureColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
        output.pixelColor.rgb = (1.0f - input.vertexColor.a) * textureColor.rgb +
            input.vertexColor.a * input.vertexColor.rgb;
        output.pixelColor.a = 1.0f;
        return output;
    }
)";

ProgramSources const VolumeFogEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const VolumeFogEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
