// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

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
