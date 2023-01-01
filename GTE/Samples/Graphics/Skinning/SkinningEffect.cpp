// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#include "SkinningEffect.h"
using namespace gte;

SkinningEffect::SkinningEffect(std::shared_ptr<ProgramFactory> const& factory)
{
    int32_t api = factory->GetAPI();
    mProgram = factory->CreateFromSources(*msVSSource[api], *msPSSource[api], "");
    if (mProgram)
    {
        mSkinningMatricesConstant = std::make_shared<ConstantBuffer>(
            4 * sizeof(Matrix4x4<float>), true);

        auto const& vshader = mProgram->GetVertexShader();
        vshader->Set("PVWMatrix", mPVWMatrixConstant);
        vshader->Set("SkinningMatrices", mSkinningMatricesConstant);
    }
}

void SkinningEffect::SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer)
{
    VisualEffect::SetPVWMatrixConstant(buffer);
    mProgram->GetVertexShader()->Set("PVWMatrix", mPVWMatrixConstant);
}


std::string const SkinningEffect::msGLSLVSSource =
R"(
    uniform PVWMatrix
    {
        mat4 pvwMatrix;
    };

    uniform SkinningMatrices
    {
        mat4 skinningMatrix0;
        mat4 skinningMatrix1;
        mat4 skinningMatrix2;
        mat4 skinningMatrix3;
    };

    layout(location = 0) in vec3 modelPosition;
    layout(location = 1) in vec4 modelColor;
    layout(location = 2) in vec4 modelWeights;
    layout(location = 0) out vec4 vertexColor;

    void main()
    {
        // This shader has a fixed number of skinning matrices per vertex. If you
        // want a number that varies with the vertex, pass in an array of skinning
        // matrices. Also pass in texture coordinates that are used as lookups
        // into the array.

        // Calculate the position by adding together a convex combination of
        // transformed positions.
        vec4 hModelPosition = vec4(modelPosition, 1.0f);
    #if GTE_USE_MAT_VEC
        vec4 position0 = (skinningMatrix0 * hModelPosition) * modelWeights.x;
        vec4 position1 = (skinningMatrix1 * hModelPosition) * modelWeights.y;
        vec4 position2 = (skinningMatrix2 * hModelPosition) * modelWeights.z;
        vec4 position3 = (skinningMatrix3 * hModelPosition) * modelWeights.w;
    #else
        vec4 position0 = (hModelPosition * skinningMatrix0) * modelWeights.x;
        vec4 position1 = (hModelPosition * skinningMatrix1) * modelWeights.y;
        vec4 position2 = (hModelPosition * skinningMatrix2) * modelWeights.z;
        vec4 position3 = (hModelPosition * skinningMatrix3) * modelWeights.w;
    #endif
        vec4 skinPosition = position0 + position1 + position2 + position3;

        // Transform the position from model space to clip space.
    #if GTE_USE_MAT_VEC
        gl_Position = pvwMatrix * skinPosition;
    #else
        gl_Position = skinPosition * pvwMatrix;
    #endif
    
        // The vertex color is passed through.
        vertexColor = modelColor;
    }
)";

std::string const SkinningEffect::msGLSLPSSource =
R"(
    layout(location = 0) in vec4 vertexColor;
    layout(location = 0) out vec4 pixelColor;

    void main()
    {
        pixelColor = vertexColor;
    }
)";

std::string const SkinningEffect::msHLSLVSSource =
R"(
    cbuffer PVWMatrix
    {
        float4x4 pvwMatrix;
    };

    cbuffer SkinningMatrices
    {
        float4x4 skinningMatrix0;
        float4x4 skinningMatrix1;
        float4x4 skinningMatrix2;
        float4x4 skinningMatrix3;
    };

    struct VS_INPUT
    {
        float3 modelPosition : POSITION;
        float4 modelColor : COLOR0;
        float4 modelWeights : TEXCOORD0;
    };

    struct VS_OUTPUT
    {
        float4 vertexColor : COLOR0;
        float4 clipPosition : SV_POSITION;
    };

    VS_OUTPUT VSMain(VS_INPUT input)
    {
        // This shader has a fixed number of skinning matrices per vertex. If you
        // want a number that varies with the vertex, pass in an array of skinning
        // matrices. Also pass in texture coordinates that are used as lookups
        // into the array.

        VS_OUTPUT output;

        // Calculate the position by adding together a convex combination of
        // transformed positions.
        float4 hModelPosition = float4(input.modelPosition, 1.0f);
        float4 position0 = mul(skinningMatrix0, hModelPosition) * input.modelWeights.x;
        float4 position1 = mul(skinningMatrix1, hModelPosition) * input.modelWeights.y;
        float4 position2 = mul(skinningMatrix2, hModelPosition) * input.modelWeights.z;
        float4 position3 = mul(skinningMatrix3, hModelPosition) * input.modelWeights.w;
        float4 skinPosition = position0 + position1 + position2 + position3;

        // Transform the position from model space to clip space.
    #if GTE_USE_MAT_VEC
        output.clipPosition = mul(pvwMatrix, skinPosition);
    #else
        output.clipPosition = mul(skinPosition, pvwMatrix);
    #endif

        // The vertex color is passed through.
        output.vertexColor = input.modelColor;
        return output;
    }
)";

std::string const SkinningEffect::msHLSLPSSource =
R"(
    struct PS_INPUT
    {
        float4 vertexColor : COLOR0;
    };

    struct PS_OUTPUT
    {
        float4 pixelColor : SV_TARGET0;
    };

    PS_OUTPUT PSMain(PS_INPUT input)
    {
        PS_OUTPUT output;
        output.pixelColor = input.vertexColor;
        return output;
    }
)";

ProgramSources const SkinningEffect::msVSSource =
{
    &msGLSLVSSource,
    &msHLSLVSSource
};

ProgramSources const SkinningEffect::msPSSource =
{
    &msGLSLPSSource,
    &msHLSLPSSource
};
