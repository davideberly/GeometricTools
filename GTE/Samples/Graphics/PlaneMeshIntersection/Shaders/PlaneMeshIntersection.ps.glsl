// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(location = 0) in vec3 inVertexColor;
layout(location = 1) noperspective in vec2 inPlaneConstant;

layout(location = 0) out vec4 pixelColor;
layout(location = 1) out vec4 planeConstant;

void main()
{
    pixelColor = vec4(inVertexColor, 1.0f);
    planeConstant = vec4(inPlaneConstant, 0.0f, 0.0f);
}
