// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03


uniform sampler1DArray mySampler1;  // 2 textures in the array
uniform sampler2DArray mySampler2;  // 2 textures in the array

layout(location = 0) in vec2 vertexTCoord;

layout(location = 0) out vec4 pixelColor;

void main()
{
    pixelColor = vec4(0, 0, 0, 0);

    vec4 tcd;

    // Sample the 1D texture array.
    tcd.xy = vec2(vertexTCoord.x, 0);
    pixelColor += texture(mySampler1, tcd.xy);
    tcd.xy = vec2(vertexTCoord.x, 1);
    pixelColor += texture(mySampler1, tcd.xy);

    // Sample the 2D texture array.
    tcd.xyz = vec3(vertexTCoord, 0);
    pixelColor += texture(mySampler2, tcd.xyz);
    tcd.xyz = vec3(vertexTCoord, 1);
    pixelColor += texture(mySampler2, tcd.xyz);

    pixelColor *= 0.25f;
};
