// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct Voxel
{
    uint index;
    uint configuration;
};

// The buffer contains only those voxels with surface in them.
StructuredBuffer<Voxel> voxels;

struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    uint index : TEXCOORD0;
    uint configuration : TEXCOORD1;
};

VS_OUTPUT VSMain (VS_INPUT input)
{
    VS_OUTPUT output;
    Voxel voxel = voxels[input.id];
    output.index = voxel.index;
    output.configuration = voxel.configuration;
    return output;
}
