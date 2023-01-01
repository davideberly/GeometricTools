// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(location = 0) in vec2 vertexBaseTCoord;
layout(location = 1) in vec2 vertexDetailTCoord;
layout(location = 2) in vec4 vertexFogInfo;

layout(location = 0) out vec4 pixelColor;

uniform sampler2D baseSampler;
uniform sampler2D detailSampler;

void main()
{
    // Sample the texture images and multiply the results.
    vec3 baseColor = texture(baseSampler, vertexBaseTCoord).xyz;
    vec3 detailColor = texture(detailSampler, vertexDetailTCoord).xyz;
    vec3 product = baseColor * detailColor;

    // Combine the base*detail color with the fog color using
    // lerp(x,y,s) = x*(1-s) + y*s.
    pixelColor.rgb = vertexFogInfo.rgb * (1.0f - vertexFogInfo.w) + product * vertexFogInfo.w;
    pixelColor.a = 1.0f;
}
