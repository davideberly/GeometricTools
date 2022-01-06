// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2022
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float4> grassTexture;
SamplerState grassSampler;  // minLmagLmipL, repeat

Texture2D<float4> stoneTexture;
SamplerState stoneSampler;  // minLmagLmipL, repeat

Texture2D<float4> cloudTexture;
SamplerState cloudSampler;  // minLmagLmipL, repeat

Texture1D<float4> blendTexture;
SamplerState blendSampler;  // minPmagLmipP, repeat

cbuffer PowerFactor
{
    float powerFactor;
};

struct PS_INPUT
{
    float2 vertexGroundTCoord : TEXCOORD0;
    float vertexBlendTCoord : TEXCOORD1;
    float2 vertexCloudTCoord : TEXCOORD2;
    float2 vertexFlowDirection : TEXCOORD3;
};

float4 PSMain (PS_INPUT input) : SV_TARGET
{
    float4 grassColor = grassTexture.Sample(grassSampler, input.vertexGroundTCoord);
    float4 stoneColor = stoneTexture.Sample(stoneSampler, input.vertexGroundTCoord);
    float4 blendColor = blendTexture.Sample(blendSampler, input.vertexBlendTCoord);

    float2 offsetCloudTCoord = input.vertexCloudTCoord + input.vertexFlowDirection;    
    float4 cloudColor = cloudTexture.Sample(cloudSampler, offsetCloudTCoord);

    float stoneWeight = pow(abs(blendColor.r), powerFactor);
    float grassWeight = 1.0f - stoneWeight;
    float4 pixelColor = cloudColor * (grassWeight * grassColor + stoneWeight * stoneColor);
    return pixelColor;
}
