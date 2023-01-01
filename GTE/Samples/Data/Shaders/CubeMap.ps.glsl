// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Reflectivity
{
    float reflectivity;
};

uniform samplerCube cubeSampler;

layout(location = 0) in vec4 vertexColor;
layout(location = 1) in vec3 cubeTCoord;

layout(location = 0) out vec4 pixelColor;

void main()
{
    vec4 reflectedColor = texture(cubeSampler, cubeTCoord);

    // In HLSL lerp(x,y,s) -> x + s*(y-x)
    // pixelColor = lerp(vertexColor, reflectedColor, reflectivity)
    float s = clamp(reflectivity, 0.0f, 1.0f);
    pixelColor = vertexColor + s * (reflectedColor - vertexColor);
}
