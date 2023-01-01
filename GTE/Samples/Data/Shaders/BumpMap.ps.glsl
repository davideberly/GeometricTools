// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform sampler2D baseSampler;
uniform sampler2D normalSampler;

layout(location = 0) in vec3 vertexLightDirection;
layout(location = 1) in vec2 vertexBaseTCoord;
layout(location = 2) in vec2 vertexNormalTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    vec3 baseColor = texture(baseSampler, vertexBaseTCoord).rgb;
    vec3 normalColor = texture(normalSampler, vertexNormalTCoord).rgb;
    vec3 lightDirection = 2.0f * vertexLightDirection - 1.0f;
    vec3 normalDirection = 2.0f * normalColor - 1.0f;
    lightDirection = normalize(lightDirection);
    normalDirection = normalize(normalDirection);
    float LdN = dot(lightDirection, normalDirection);
    LdN = clamp(LdN, 0.0f, 1.0f);
    pixelColor = vec4(LdN * baseColor, 1.0f);
}
