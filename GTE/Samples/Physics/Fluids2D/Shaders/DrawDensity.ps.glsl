// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform sampler2D stateSampler;

layout(location = 0) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    // Map velocity channels to colors and modulate by density.
    vec4 current = texture(stateSampler, vertexTCoord);
    vec3 color = 0.5f + 0.5f * current.xyz / (1.0f + abs(current.xyz));
    pixelColor = vec4(current.w * color, 1.0f);
}
