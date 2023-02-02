// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture1DArray myTexture1;  // 2 textures in the array
Texture2DArray myTexture2;  // 2 textures in the array
SamplerState mySampler1;
SamplerState mySampler2;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor = 0.0f;

    float4 tcd;

    // Sample the 1D texture array.
    tcd.xy = float2(input.vertexTCoord.x, 0);
    output.pixelColor += myTexture1.Sample(mySampler1, tcd.xy);
    tcd.xy = float2(input.vertexTCoord.x, 1);
    output.pixelColor += myTexture1.Sample(mySampler1, tcd.xy);

    // Sample the 2D texture array.
    tcd.xyz = float3(input.vertexTCoord, 0);
    output.pixelColor += myTexture2.Sample(mySampler2, tcd.xyz);
    tcd.xyz = float3(input.vertexTCoord, 1);
    output.pixelColor += myTexture2.Sample(mySampler2, tcd.xyz);

    output.pixelColor *= 0.25f;
    return output;
};
