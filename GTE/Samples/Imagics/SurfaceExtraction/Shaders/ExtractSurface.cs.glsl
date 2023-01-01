// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Parameters
{
    vec3 delta;  // (dx,dy,dz) voxel dimensions
    float levelValue;
};

const vec3 corner[8] =
{
    vec3(0.0f, 0.0f, 0.0f),
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(0.0f, 0.0f, 1.0f),
    vec3(1.0f, 0.0f, 1.0f),
    vec3(0.0f, 1.0f, 1.0f),
    vec3(1.0f, 1.0f, 1.0f)
};

struct Voxel
{
    uint configuration;
    uint numVertices;
    uint numTriangles;
    vec3 vertices[12];
    uint indices[15];
};

// Marching Cubes lookupSB table for mesh topology within a voxel.  This table
// is of the form Table[256][41], but is stored as Lookup[10496].
// HLSL: StructuredBuffer<uint> lookup;
//
buffer lookup { uint data[]; } lookupSB;

// XSIZE-by-YSIZE-by-ZSIZE image stored in lexicographical order
// HLSL: StructuredBuffer<float> image;
//
buffer image { float data[]; } imageSB;

// Output only those voxels with surface in them.
// HLSL: AppendStructuredBuffer<Voxel> voxels;
//
buffer voxels { Voxel data[]; } voxelsAC;
layout(binding = 0, offset = 0) uniform atomic_uint voxelsCounter;
void voxelsAppend(Voxel v)
{
    uint index = atomicCounterIncrement(voxelsCounter);
    voxelsAC.data[index] = v;
}


// Process a DIM-by-DIM-by-DIM block of voxels, make the call
// Dispatch(XSIZE/XTHREADS, YSIZE/YTHREADS, ZSIZE/ZTHREADS).
layout (local_size_x = XTHREADS, local_size_y = YTHREADS, local_size_z = ZTHREADS) in;
void main()
{
    // HLSL: void CSMain (uint3 gtID : SV_GroupThreadID, uint3 gID : SV_GroupID)
    uvec3 gID = gl_WorkGroupID;
    uvec3 gtID = gl_LocalInvocationID;

    // gID in [0..XSIZE),[0..YSIZE)x[0,ZSIZE), gtID in [0,1]^3
    uint x = XTHREADS*gID.x + gtID.x;
    uint y = YTHREADS*gID.y + gtID.y;
    uint z = ZTHREADS*gID.z + gtID.z;
    uint index = x + XBOUND*(y + YBOUND*z);

    if (x+1 < XBOUND && y+1 < YBOUND && z+1 < ZBOUND)
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
            imageSB.data[i000] - levelValue,
            imageSB.data[i100] - levelValue,
            imageSB.data[i010] - levelValue,
            imageSB.data[i110] - levelValue,
            imageSB.data[i001] - levelValue,
            imageSB.data[i101] - levelValue,
            imageSB.data[i011] - levelValue,
            imageSB.data[i111] - levelValue
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
            vec3 origin = vec3(x, y, z);
            uint j = 41*configuration;
            Voxel voxel;
            voxel.configuration = configuration;
            voxel.numVertices = lookupSB.data[j++];
            voxel.numTriangles = lookupSB.data[j++];
            for (i = 0; i < 12; ++i)
            {
                uint j0 = lookupSB.data[j++];
                uint j1 = lookupSB.data[j++];

                float invDenom =
                    (i < voxel.numVertices ? 1.0f/(F[j0]-F[j1]) : 0.0f);
                vec3 numer = F[j0]*corner[j1] - F[j1]*corner[j0];
                voxel.vertices[i] = delta*(origin + numer*invDenom);
            }

            for (i = 0; i < 5; ++i)
            {
                voxel.indices[i*3+0] = lookupSB.data[j++];
                voxel.indices[i*3+1] = lookupSB.data[j++];
                voxel.indices[i*3+2] = lookupSB.data[j++];
            }

            voxelsAppend(voxel);
        }
    }
}
