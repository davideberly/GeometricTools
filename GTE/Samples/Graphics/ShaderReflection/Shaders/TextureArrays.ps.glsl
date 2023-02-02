// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform sampler2DArray mySampler;

layout(location = 0) in vec2 vertexTCoord;

layout(location = 0) out vec4 pixelColor;

void main()
{
    vec3 tcoord0 = vec3(vertexTCoord, 0);
    vec4 color0 = texture(mySampler, tcoord0);
    vec3 tcoord1 = vec3(vertexTCoord, 1);
    vec4 color1 = texture(mySampler, tcoord1);
    pixelColor = 0.5f * (color0 + color1);
};
