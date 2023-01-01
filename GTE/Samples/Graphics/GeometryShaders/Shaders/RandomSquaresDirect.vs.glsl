// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColorSize;
layout(location = 0) out vec4 outColorSize;

void main()
{
    gl_Position = inPosition;
    outColorSize = inColorSize;
}
