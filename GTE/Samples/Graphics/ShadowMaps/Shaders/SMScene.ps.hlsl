// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

cbuffer LightColor
{
    float4 lightColor;
};

Texture2D baseTexture;
SamplerState baseSampler;

Texture2D blurTexture;
SamplerState blurSampler;

Texture2D projTexture;
SamplerState projSampler;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float4 projTCoord : TEXCOORD1;
    float4 screenTCoord : TEXCOORD2;
    float3 vertexNormal : TEXCOORD3;
    float3 lightVector : TEXCOORD4;
    float3 eyeVector : TEXCOORD5;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;

    // Normalize the input vectors.
    input.vertexNormal = normalize(input.vertexNormal);
    input.lightVector = normalize(input.lightVector);
    input.eyeVector = normalize(input.eyeVector);

    // Get the base color.
    float4 baseColor = baseTexture.Sample(baseSampler, input.vertexTCoord);

    // Compute the ambient lighting term (zero, for this example).
    float ambient = 0.0f;

    // Compute the diffuse lighting term.
    float NdL = dot(input.vertexNormal, input.lightVector);
    float diffuse = max(NdL, 0.0f);

    // Compute the specular lighting term.
    float specular;
    if (diffuse != 0.0f)
    {
        float3 tmp = 2.0f * NdL * input.vertexNormal - input.lightVector;
        specular = pow(max(dot(tmp, input.lightVector), 0.0f), 8.0f);
    }
    else
    {
        specular = 0.0f;
    }

    // Clamp the spot texture to a disk centered in the texture in order to
    // give the appearance of a spotlight cone.
    float u = input.projTCoord.x / input.projTCoord.w;
    float v = input.projTCoord.y / input.projTCoord.w;
    float du = u - 0.5f;
    float dv = v - 0.5f;
    float rsqr = du * du + dv * dv;
    float weight = (rsqr <= 0.25f ? 0.0f : 1.0f);

    float2 blurTCoord = input.screenTCoord.xy / input.screenTCoord.w;
    float shadow = blurTexture.Sample(blurSampler, blurTCoord).r;
    float4 projColor = projTexture.Sample(projSampler, float2(u, v));
    float4 shadowColor = shadow * projColor;
    float4 ambientColor = ambient * baseColor;
    float4 diffuseColor = diffuse * baseColor * lightColor;
    float4 specularColor = specular * baseColor * lightColor.a;

    output.pixelColor = ambientColor + (diffuseColor + specularColor) * (
        weight + (1.0f - weight) * shadowColor);

    output.pixelColor.a = 1.0f;

    return output;
}
