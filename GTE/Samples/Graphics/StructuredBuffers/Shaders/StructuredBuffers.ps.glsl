// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform sampler2D baseSampler;

buffer drawnPixels
{
    vec4 data[];
} drawnPixelsSB;

layout(location = 0) in vec2 vertexTCoord;

layout(location = 0) out vec4 pixelColor0;

void main()
{
    vec4 outputColor = texture(baseSampler, vertexTCoord);
    pixelColor0 = outputColor;
    ivec2 location = ivec2(gl_FragCoord.xy);
    drawnPixelsSB.data[location.x + WINDOW_WIDTH * location.y] =  outputColor;
};
