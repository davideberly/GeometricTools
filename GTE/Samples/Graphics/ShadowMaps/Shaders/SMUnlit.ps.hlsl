// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

cbuffer Screen
{
    float4 screen;  // (depthBias, txSize, tySize, unused)
};

Texture2D shadowTexture;
SamplerState shadowSampler;

struct PS_INPUT
{
    float4 projTCoord : TEXCOORD0;
    float depth : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;

    // Generate the texture coordinates for the specified depth-map size.
    float txSize = screen.y;
    float tySize = screen.z;
    float4 tcoords[9];
    tcoords[0] = input.projTCoord;
    tcoords[1] = input.projTCoord + float4(-txSize,    0.0f, 0.0f, 0.0f);
    tcoords[2] = input.projTCoord + float4(+txSize,    0.0f, 0.0f, 0.0f);
    tcoords[3] = input.projTCoord + float4(0.0f,    -tySize, 0.0f, 0.0f);
    tcoords[4] = input.projTCoord + float4(-txSize, -tySize, 0.0f, 0.0f);
    tcoords[5] = input.projTCoord + float4(+txSize, -tySize, 0.0f, 0.0f);
    tcoords[6] = input.projTCoord + float4(0.0f,    +tySize, 0.0f, 0.0f);
    tcoords[7] = input.projTCoord + float4(-txSize, +tySize, 0.0f, 0.0f);
    tcoords[8] = input.projTCoord + float4(+txSize, +tySize, 0.0f, 0.0f);
    float w = input.projTCoord.w;

    // Sample each of them, checking whether or not the pixel is shadowed.
    float depthBias = screen.x;
    float diff = input.depth - depthBias;
    float shadowTerm = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        tcoords[i] /= w;
        float rvalue = shadowTexture.Sample(shadowSampler, tcoords[i].xy).r;
        if (rvalue >= diff)
        {
            // The pixel is not in shadow.
            shadowTerm += 1.0f;
        }
    }
    shadowTerm /= 9.0f;
    output.pixelColor = shadowTerm;

    return output;
}
