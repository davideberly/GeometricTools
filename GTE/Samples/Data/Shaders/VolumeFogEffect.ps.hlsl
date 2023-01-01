// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

Texture2D baseTexture;
SamplerState baseSampler;

struct PS_INPUT
{
    float4 vertexColor : COLOR0;
    float2 vertexTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    // The blending equation is
    //   (rf,gf,bf) = (1-av)*(rt,gt,bt) + av*(rv,gv,bv)
    // where (rf,gf,bf) is the final color, (rt,gt,bt) is the texture color,
    // and (rv,gv,bv,av) is the vertex color.

    PS_OUTPUT output;
    float4 textureColor = baseTexture.Sample(baseSampler, input.vertexTCoord);
    output.pixelColor.rgb = (1.0f - input.vertexColor.a) * textureColor.rgb +
        input.vertexColor.a * input.vertexColor.rgb;
    output.pixelColor.a = 1.0f;
    return output;
}
