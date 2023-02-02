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
    float4 lightWorldPosition;
    float4 cameraWorldPosition;
};

static float4x4 lightBSMatrix =
{
    0.5f,  0.0f, 0.0f, 0.5f,
    0.0f, -0.5f, 0.0f, 0.5f,
    0.0f,  0.0f, 1.0f, 0.0f,
    0.0f,  0.0f, 0.0f, 1.0f
};

static float4x4 screenBSMatrix =
{
    0.5f,  0.0f, 0.0f, 0.5f,
    0.0f, -0.5f, 0.0f, 0.5f,
    0.0f,  0.0f, 1.0f, 0.0f,
    0.0f,  0.0f, 0.0f, 1.0f
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float3 modelNormal : NORMAL;
    float2 modelTCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float4 projTCoord : TEXCOORD1;
    float4 screenTCoord : TEXCOORD2;
    float3 vertexNormal : TEXCOORD3;
    float3 lightVector : TEXCOORD4;
    float3 eyeVector : TEXCOORD5;
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

    // Pass through the texture coordinates.
    output.vertexTCoord = input.modelTCoord;

    // Transform the position from model space to world space.
#if GTE_USE_MAT_VEC
    float4 worldPosition = mul(worldMatrix, hModelPosition);
#else
    float4 worldPosition = mul(hModelPosition, worldMatrix);
#endif

    // Transform the normal from model space to world space.
    float4 hModelNormal = float4(input.modelNormal, 0.0f);
#if GTE_USE_MAT_VEC
    output.vertexNormal = mul(worldMatrix, hModelNormal).xyz;
#else
    output.vertexNormal = mul(hModelNormal, worldMatrix).xyz;
#endif

    // Compute the projected texture coordinates.
#if GTE_USE_MAT_VEC
    float4 lightSpacePosition = mul(lightPVMatrix, worldPosition);
    output.projTCoord = mul(lightBSMatrix, lightSpacePosition);
#else
    float4 lightSpacePosition = mul(worldPosition, lightPVMatrix);
    output.projTCoord = mul(lightSpacePosition, lightBSMatrix);
#endif

    // Compute the screen-space texture coordinates.
#if GTE_USE_MAT_VEC
    output.screenTCoord = mul(screenBSMatrix, output.clipPosition);
#else
    output.screenTCoord = mul(output.clipPosition, screenBSMatrix);
#endif

    // Transform the light vector to tangent space.
    output.lightVector = lightWorldPosition.xyz - worldPosition.xyz;

    // Transform the eye vector into tangent space.
    output.eyeVector = cameraWorldPosition.xyz - worldPosition.xyz;

    return output;
}
