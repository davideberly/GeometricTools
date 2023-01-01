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

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
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
    uint index;
    uint configuration;
};

// Marching Cubes lookup table for mesh topology within a voxel.  This table
// is of the form Table[256][41], but is stored as Lookup[10496].
StructuredBuffer<uint> lookup;

// XSIZE-by-YSIZE-by-ZSIZE image stored in lexicographical order
StructuredBuffer<float> image;

// The buffer contains only those voxels with surface in them.
StructuredBuffer<Voxel> voxels;

struct GS_INPUT
{
    uint index : TEXCOORD0;
    uint configuration : TEXCOORD1;
};

struct GS_OUTPUT
{
    float4 clipPosition : SV_POSITION;
    float3 tcoord : TEXCOORD0;
};

[maxvertexcount(15)]
void GSMain (point GS_INPUT voxel[1], inout TriangleStream<GS_OUTPUT> stream)
{
    uint x = voxel[0].index % XBOUND;
    uint q = voxel[0].index / XBOUND;
    uint y = q % YBOUND;
    uint z = q / YBOUND;
    float3 origin = float3(x, y, z);

    uint i000 = voxel[0].index;
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

    uint j = 41 * voxel[0].configuration;
    uint numVertices = lookup[j++];
    uint numTriangles = lookup[j++];

    // The position[] array must be initialized to avoid
    //    error X4580: emitting a system-interpreted value which is not
    //    written in every execution path of the shader
    // This error points to the stream.Append(output) call, so the HLSL
    // compiler cannot deduce when we are using only valid values in
    // position[].  This should be a warning, not an error.
    float3 position[12] =
    {
        float3(0,0,0), float3(0,0,0), float3(0,0,0), float3(0,0,0),
        float3(0,0,0), float3(0,0,0), float3(0,0,0), float3(0,0,0),
        float3(0,0,0), float3(0,0,0), float3(0,0,0), float3(0,0,0)
    };

    uint i;
    for (i = 0; i < numVertices; ++i)
    {
        uint j0 = lookup[j++];
        uint j1 = lookup[j++];
        float3 offset = (F[j0] * corner[j1] - F[j1] * corner[j0]) / (F[j0] - F[j1]);
        position[i] = delta * (origin + offset);
    }
    j += 2 * (12 - numVertices);  // Jump to the start of the triangle indices.

    GS_OUTPUT output;
    for (i = 0; i < numTriangles; ++i)
    {
        [unroll]
        for (uint k = 0; k < 3; ++k)
        {
            uint v = lookup[j++];
#if GTE_USE_MAT_VEC
            output.clipPosition = mul(pvwMatrix, float4(position[v], 1.0f));
#else
            output.clipPosition = mul(float4(position[v], 1.0f), pvwMatrix);
#endif
            output.tcoord = 0.5f * position[v];
            stream.Append(output);
        }
        stream.RestartStrip();
    }
}
