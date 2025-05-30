// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float3 modelLightDirection : COLOR;
    float2 modelBaseTCoord : TEXCOORD0;
    float2 modelNormalTCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
    float3 vertexLightDirection : COLOR;
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexNormalTCoord : TEXCORD1;
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

    output.vertexLightDirection = input.modelLightDirection;
    output.vertexBaseTCoord = input.modelBaseTCoord;
    output.vertexNormalTCoord = input.modelNormalTCoord;
    return output;
}

