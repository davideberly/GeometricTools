// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

uniform sampler3D volumeSampler;

layout(location = 0) in vec3 vertexTCoord;

layout(location = 0) out vec4 pixelColor0;

void main()
{
    vec4 color = texture(volumeSampler, vertexTCoord);
    pixelColor0 = vec4(color.w * color.rgb, 0.5f * color.w);
};


