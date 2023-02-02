// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer VWMatrix
{
    float4x4 vwMatrix;
};

cbuffer FogColorDensity
{
    float4 fogColorDensity;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float2 modelBaseTCoord : TEXCOORD0;
    float2 modelDetailTCoord : TEXCOORD1;
};

struct VS_OUTPUT
{
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexDetailTCoord : TEXCOORD1;
    float4 vertexFogInfo : TEXCOORD2;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to clip space.
    float4 modelPosition = float4(input.modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    float3 viewPosition = mul(vwMatrix, modelPosition).xyz;
    output.clipPosition = mul(pvwMatrix, modelPosition);
#else
    float3 viewPosition = mul(modelPosition, vwMatrix).xyz;
    output.clipPosition = mul(modelPosition, pvwMatrix);
#endif

    // Transform the position from model space to view space.  This is the
    // vector from the view-space eye position (the origin) to the view-space
    // vertex position.  The fog factor (vertexFogInfo.w) uses the
    // z-component of this vector, which is z-based depth, not range-based
    // depth.
    float fogSqrDistance = dot(viewPosition, viewPosition);
    float fogExpArg = -fogColorDensity.w * fogColorDensity.w * fogSqrDistance;
    output.vertexFogInfo.rgb = fogColorDensity.rgb;
    output.vertexFogInfo.w = exp(fogExpArg);

    // Pass through the texture coordinates.
    output.vertexBaseTCoord = input.modelBaseTCoord;
    output.vertexDetailTCoord = input.modelDetailTCoord;
    return output;
}
