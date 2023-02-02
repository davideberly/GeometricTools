// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Reflectivity
{
    float reflectivity;
};

TextureCube<float4> cubeTexture;
SamplerState cubeSampler;

struct PS_INPUT
{
    float4 vertexColor : COLOR;
    float3 cubeTCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 pixelColor : SV_TARGET;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    float4 reflectedColor = cubeTexture.Sample(cubeSampler, input.cubeTCoord);
    output.pixelColor = lerp(input.vertexColor, reflectedColor, reflectivity);
    return output;
}
