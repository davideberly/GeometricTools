// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

struct VS_STRUCT
{
    float4 position : POSITION;
    float4 colorSize : COLOR0;
};

VS_STRUCT VSMain (VS_STRUCT input)
{
    return input;
}

