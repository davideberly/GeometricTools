// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Parameters
{
    float3 delta;  // (dx,dy,dz) voxel dimensions
    float levelValue;
};

static float3 corner[8] =
{
    float3(0.0f, 0.0f, 0.0f),
    float3(1.0f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(1.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f),
    float3(1.0f, 0.0f, 1.0f),
    float3(0.0f, 1.0f, 1.0f),
    float3(1.0f, 1.0f, 1.0f)
};

struct Voxel
{
    uint configuration;
    uint numVertices;
    uint numTriangles;
    uint unused0;
    float4 vertices[12];
    uint3 indices[5];
    uint unused1;
};

// Marching Cubes lookup table for mesh topology within a voxel.  This table
// is of the form Table[256][41], but is stored as Lookup[10496].
StructuredBuffer<uint> lookup;

// XSIZE-by-YSIZE-by-ZSIZE image stored in lexicographical order
StructuredBuffer<float> image;

// Output only those voxels with surface in them.
AppendStructuredBuffer<Voxel> voxels;


// Process a DIM-by-DIM-by-DIM block of voxels, make the call
// Dispatch(XSIZE/XTHREADS, YSIZE/YTHREADS, ZSIZE/ZTHREADS).
[numthreads(XTHREADS, YTHREADS, ZTHREADS)]
void CSMain (uint3 gtID : SV_GroupThreadID, uint3 gID : SV_GroupID)
{
    // gID in [0..XSIZE),[0..YSIZE)x[0,ZSIZE), gtID in [0,1]^3
    uint x = XTHREADS * gID.x + gtID.x;
    uint y = YTHREADS * gID.y + gtID.y;
    uint z = ZTHREADS * gID.z + gtID.z;
    uint index = x + XBOUND * (y + YBOUND * z);

    if (x + 1 < XBOUND && y + 1 < YBOUND && z + 1 < ZBOUND)
    {
        uint i000 = x + XBOUND*(y + YBOUND*z);
        uint i100 = i000 + 1;
        uint i010 = i000 + XBOUND;
        uint i110 = i010 + 1;
        uint i001 = i000 + XBOUND*YBOUND;
        uint i101 = i001 + 1;
        uint i011 = i001 + XBOUND;
        uint i111 = i011 + 1;
        float F[8] =
        {
            image[i000] - levelValue,
            image[i100] - levelValue,
            image[i010] - levelValue,
            image[i110] - levelValue,
            image[i001] - levelValue,
            image[i101] - levelValue,
            image[i011] - levelValue,
            image[i111] - levelValue
        };

        uint configuration = 0;
        uint i, mask;
        for (i = 0, mask = 1; i < 8; ++i, mask <<= 1)
        {
            if (F[i] < 0.0f)
            {
                configuration |= mask;
            }
        }

        if (0 < configuration && configuration < 255)
        {
            float3 origin = float3(x, y, z);
            uint j = 41*configuration;
            Voxel voxel;
            voxel.configuration = configuration;
            voxel.numVertices = lookup[j++];
            voxel.numTriangles = lookup[j++];
            voxel.unused0 = 0;
            voxel.unused1 = 0;
            [unroll]
            for (i = 0; i < 12; ++i)
            {
                uint j0 = lookup[j++];
                uint j1 = lookup[j++];

                float invDenom = (i < voxel.numVertices ? 1.0f / (F[j0] - F[j1]) : 0.0f);
                float3 numer = F[j0] * corner[j1] - F[j1] * corner[j0];
                voxel.vertices[i].xyz = delta * (origin + numer * invDenom);
                voxel.vertices[i].w = 1.0f;
            }

            [unroll]
            for (i = 0; i < 5; ++i)
            {
                voxel.indices[i].x = lookup[j++];
                voxel.indices[i].y = lookup[j++];
                voxel.indices[i].z = lookup[j++];
            }

            voxels.Append(voxel);
        }
    }
}
