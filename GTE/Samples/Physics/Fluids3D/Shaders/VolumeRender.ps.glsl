// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform sampler3D volumeSampler;

layout(location = 0) in vec3 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

void main()
{
    vec4 color = texture(volumeSampler, vertexTCoord);
    pixelColor = vec4(color.w * color.rgb, 0.5f * color.w);
}
