// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

layout(location = 0) in float depth;
layout(location = 0) out vec4 pixelColor;

void main()
{
    pixelColor = vec4(depth, depth, depth, depth);
}
