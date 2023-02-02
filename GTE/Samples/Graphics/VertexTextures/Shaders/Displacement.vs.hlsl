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
    displacedPosition.z = displacementTexture.SampleLevel(displacementSampler, input.modelTCoord, 0);
    displacedPosition.w = 1.0f;

    output.vertexHeight = displacedPosition.z;

#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, displacedPosition);
#else
    output.clipPosition = mul(displacedPosition, pvwMatrix);
#endif

    return output;
}
