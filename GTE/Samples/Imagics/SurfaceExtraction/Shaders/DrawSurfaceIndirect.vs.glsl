// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

struct Voxel
{
    uint index;
    uint configuration;
};

// The buffer contains only those voxels with surface in them.
// HLSL: StructuredBuffer<Voxel> voxels;
//
buffer voxels { Voxel data[]; } voxelsSB;

out Vertex
{
    uint index;
    uint configuration;
} outVertex;

void main()
{
    Voxel voxel = voxelsSB.data[gl_VertexID];
    outVertex.index = voxel.index;
    outVertex.configuration = voxel.configuration;
}

