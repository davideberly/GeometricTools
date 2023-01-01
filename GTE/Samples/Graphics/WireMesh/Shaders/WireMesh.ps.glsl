// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform WireParameters
{
    vec4 meshColor;
    vec4 edgeColor;
    vec2 windowSize;
};

layout(location = 0) in vec4 pixelColor;
layout(location = 1) noperspective in vec3 edgeDistance;
layout(location = 0) out vec4 finalColor;

void main()
{
    float dmin = min(min(edgeDistance[0], edgeDistance[1]), edgeDistance[2]);
    float blend = smoothstep(0.0f, 1.0f, dmin);
    finalColor = mix(edgeColor, pixelColor, blend);
    finalColor.a = dmin;
}
