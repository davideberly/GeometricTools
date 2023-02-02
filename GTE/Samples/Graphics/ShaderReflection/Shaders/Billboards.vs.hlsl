// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct VS_INPUT
{
    float3 position : POSITION;
    float3 color : COLOR0;
    float size : TEXCOORD0;
};

struct VS_OUTPUT
{
    float3 position : POSITION;
    float3 color : COLOR0;
    float size : TEXCOORD0;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    return input;
}
