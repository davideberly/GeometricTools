// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float> inImage;
Texture2D<float> inMask;
SamplerState imageSampler;
SamplerState maskSampler;

struct PS_INPUT
{
    float2 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

static float4 boundaryColor = { 0.0f, 1.0f, 0.0f, 1.0f };

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float image = inImage.Sample(imageSampler, input.vertexTCoord);
    float4 interiorColor = { image, image, image, 1.0f };
    float mask = inMask.Sample(maskSampler, input.vertexTCoord);
    output.pixelColor = mask * interiorColor + (1.0f - mask) * boundaryColor;
    return output;
};
