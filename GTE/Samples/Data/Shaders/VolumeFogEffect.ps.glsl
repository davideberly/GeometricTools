// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

uniform sampler2D baseSampler;

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    vec4 textureColor = texture(baseSampler, vertexTCoord);
    pixelColor.rgb = (1.0f - vertexColor.a) * textureColor.rgb +
        vertexColor.a * vertexColor.rgb;
    pixelColor.a = 1.0f;
}
