// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10
    
layout(location = 0) in vec4 vertexColor;
layout(location = 0) out vec4 pixelColor;

void main()
{
    pixelColor = vertexColor;
}


