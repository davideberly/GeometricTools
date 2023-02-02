// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer Geometry
{
    float4x4 worldMatrix;
    float4x4 lightPVMatrix;
};

static float4x4 lightBSMatrix =
{
    0.5f,  0.0f, 0.0f, 0.5f,
    0.0f, -0.5f, 0.0f, 0.5f,
    0.0f,  0.0f, 1.0f, 0.0f,
    0.0f,  0.0f, 0.0f, 1.0f
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
};

struct VS_OUTPUT
{
    float4 projTCoord : TEXCOORD0;
    float depth : TEXCOORD1;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to clip space.
    float4 hModelPosition = float4(input.modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, hModelPosition);
#else
    output.clipPosition = mul(hModelPosition, pvwMatrix);
#endif

    // Transform the position from model space to light space.
#if GTE_USE_MAT_VEC
    float4 worldPosition = mul(worldMatrix, hModelPosition);
    float4 lightSpacePosition = mul(lightPVMatrix, worldPosition);
#else
    float4 worldPosition = mul(hModelPosition, worldMatrix);
    float4 lightSpacePosition = mul(worldPosition, lightPVMatrix);
#endif

    // Compute the projected texture coordinates.
#if GTE_USE_MAT_VEC
    output.projTCoord = mul(lightBSMatrix, lightSpacePosition);
#else
    output.projTCoord = mul(lightSpacePosition, lightBSMatrix);
#endif

    // Output the distance from the light source.
    output.depth = lightSpacePosition.z;

    return output;
}
