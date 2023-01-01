// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float4> baseTexture;
SamplerState baseSampler;

Texture2D<float4> normalTexture;
SamplerState normalSampler;

struct PS_INPUT
{
    float3 vertexLightDirection : COLOR;
    float2 vertexBaseTCoord : TEXCOORD0;
    float2 vertexNormalTCoord : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float3 baseColor = baseTexture.Sample(baseSampler, input.vertexBaseTCoord).rgb;
    float3 normalColor = normalTexture.Sample(normalSampler, input.vertexNormalTCoord).rgb;
    float3 lightDirection = 2.0f * input.vertexLightDirection - 1.0f;
    float3 normalDirection = 2.0f * normalColor - 1.0f;
    lightDirection = normalize(lightDirection);
    normalDirection = normalize(normalDirection);
    float LdN = dot(lightDirection, normalDirection);
    LdN = saturate(LdN);
    output.pixelColor = float4(LdN * baseColor, 1.0f);
    return output;
}
