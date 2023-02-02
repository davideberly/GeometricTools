// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float4> baseTexture;
SamplerState baseSampler;
RWStructuredBuffer<float4> drawnPixels;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
    float4 location : SV_POSITION;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
    int2 location = int2(input.location.xy);
    drawnPixels[location.x + WINDOW_WIDTH * location.y] =  output.pixelColor;
    return output;
};
