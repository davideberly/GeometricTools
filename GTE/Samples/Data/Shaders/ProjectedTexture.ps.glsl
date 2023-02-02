// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec4 projectorTCoord;

layout(location = 0) out vec4 pixelColor;

uniform sampler2D baseSampler;

void main()
{
    vec2 tcoord = projectorTCoord.xy / projectorTCoord.w;
    vec4 baseColor = texture(baseSampler, tcoord);
    pixelColor = baseColor * vertexColor;
}
