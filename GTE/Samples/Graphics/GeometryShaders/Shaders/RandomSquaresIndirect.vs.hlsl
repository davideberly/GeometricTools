// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    uint id : TEXCOORD0;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;
    output.id = input.id;
    return output;
}


