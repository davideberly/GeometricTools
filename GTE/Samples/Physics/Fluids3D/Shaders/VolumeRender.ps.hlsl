// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture3D<float4> volumeTexture;
SamplerState volumeSampler;

struct PS_INPUT
{
    float3 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float4 color = volumeTexture.Sample(volumeSampler, input.vertexTCoord);
    output.pixelColor = float4(color.w * color.rgb, 0.5f * color.w);
    return output;
};
