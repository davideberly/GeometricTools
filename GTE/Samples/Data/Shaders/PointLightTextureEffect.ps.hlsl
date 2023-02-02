// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Material
{
    float4 materialEmissive;
    float4 materialAmbient;
    float4 materialDiffuse;
    float4 materialSpecular;
};

cbuffer Lighting
{
    float4 lightingAmbient;
    float4 lightingDiffuse;
    float4 lightingSpecular;
    float4 lightingAttenuation;
};

cbuffer LightCameraGeometry
{
    float4 lightModelPosition;
    float4 cameraModelPosition;
};

Texture2D<float4> baseTexture;
SamplerState baseSampler;

struct PS_INPUT
{
    float3 vertexPosition : TEXCOORD0;
    float3 vertexNormal : TEXCOORD1;
    float2 vertexTCoord : TEXCOORD2;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;

    float3 modelLightDiff = input.vertexPosition - lightModelPosition.xyz;
    float3 vertexDirection = normalize(modelLightDiff);
    float NDotL = -dot(input.vertexNormal, vertexDirection);
    float3 viewVector = normalize(cameraModelPosition.xyz - input.vertexPosition.xyz);
    float3 halfVector = normalize(viewVector - vertexDirection);
    float NDotH = dot(input.vertexNormal, halfVector);
    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    float3 lightingColor = materialAmbient.rgb * lightingAmbient.rgb +
        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

    float distance = length(modelLightDiff);
    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
        (lightingAttenuation.y + distance * lightingAttenuation.z));

    float4 textureColor = baseTexture.Sample(baseSampler, input.vertexTCoord);

    float3 color = lightingColor * textureColor.rgb;
    output.pixelColor.rgb = materialEmissive.rgb + attenuation * color;
    output.pixelColor.a = materialDiffuse.a * textureColor.a;
    return output;
}
