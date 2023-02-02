// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

cbuffer Geometry
{
    float4x4 worldMatrix;
    float4x4 lightPVMatrix;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
};

struct VS_OUTPUT
{
    float depth : TEXCOORD0;
    float4 lightSpacePosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    // Transform the position from model space to light space.
    float4 hModelPosition = float4(input.modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    float4 worldPosition = mul(worldMatrix, hModelPosition);
    output.lightSpacePosition = mul(lightPVMatrix, worldPosition);
#else
    float4 worldPosition = mul(hModelPosition, worldMatrix);
    output.lightSpacePosition = mul(worldPosition, lightPVMatrix);
#endif

    // Output the distance from the light source to the vertex.
    output.depth = output.lightSpacePosition.z;

    return output;
}
