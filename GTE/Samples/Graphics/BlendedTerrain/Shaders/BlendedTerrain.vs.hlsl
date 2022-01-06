// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer FlowDirection
{
    float2 flowDirection;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelGroundTCoord : TEXCOORD0;
    float modelBlendTCoord : TEXCOORD1;
    float2 modelCloudTCoord : TEXCOORD2;
};

struct VS_OUTPUT
{
    float2 vertexGroundTCoord : TEXCOORD0;
    float vertexBlendTCoord : TEXCOORD1;
    float2 vertexCloudTCoord : TEXCOORD2;
    float2 vertexFlowDirection : TEXCOORD3;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to clip space.
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
#else
    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
#endif

    // Pass through the texture coordinates.
    output.vertexGroundTCoord = input.modelGroundTCoord;
    output.vertexBlendTCoord = input.modelBlendTCoord;
    output.vertexCloudTCoord = input.modelCloudTCoord;

    // Pass through the flow direction, to be used as an offset in the pixel
    // program.
    output.vertexFlowDirection = flowDirection;
    return output;
}
