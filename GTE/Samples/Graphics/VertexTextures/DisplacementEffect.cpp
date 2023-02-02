// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include "DisplacementEffect.h"
using namespace gte;

DisplacementEffect::DisplacementEffect(std::shared_ptr<ProgramFactory> const& factory,
    std::shared_ptr<Texture2> const& texture,
    SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1)
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

        auto const& vshader = mProgram->GetVertexShader();
        vshader->Set("PVWMatrix", mPVWMatrixConstant);
        vshader->Set("displacementTexture", texture, "displacementSampler", mSampler);
    }
}

void DisplacementEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const DisplacementEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec2 modelTCoord;
    layout(location = 0) out float vertexHeight;

    uniform sampler2D displacementSampler;

    void main()
    {
        vec4 displacedPosition;
        displacedPosition.xy = modelPosition.xy;
        displacedPosition.z = textureLod(displacementSampler, modelTCoord, 0).x;
        displacedPosition.w = 1.0f;

        vertexHeight = displacedPosition.z;

    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * displacedPosition;
    #else
        gl_Position = displacedPosition * pvwMatrix;
    #endif
    }
)";

std::string const DisplacementEffect::msGLSLPSSource =
R"(
    layout(location = 0) in float vertexHeight;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor.rgb = vec3(vertexHeight);
        pixelColor.a = 1.0f;
    }
)";

std::string const DisplacementEffect::msHLSLVSSource =
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
        float vertexHeight : TEXCOORD0;
        float4 clipPosition : SV_POSITION;
    };

    Texture2D<float> displacementTexture;
    SamplerState displacementSampler;

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        VS_OUTPUT output;

        float4 displacedPosition;
        displacedPosition.xy = input.modelPosition.xy;
        displacedPosition.z = displacementTexture.SampleLevel(
            displacementSampler, input.modelTCoord, 0);
        displacedPosition.w = 1.0f;

        output.vertexHeight = displacedPosition.z;

    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, displacedPosition);
    #else
        output.clipPosition = mul(displacedPosition, pvwMatrix);
    #endif

        return output;
    }
)";

std::string const DisplacementEffect::msHLSLPSSource =
R"(
    struct PS_INPUT
    {
        float vertexHeight : TEXCOORD0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        output.pixelColor.rgb = input.vertexHeight;
        output.pixelColor.a = 1.0f;
        return output;
    }
)";

ProgramSources const DisplacementEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const DisplacementEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
