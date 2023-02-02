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

uniform PVWMatrix
{
    mat4 pvwMatrix;
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

// Marching Cubes lookup table for mesh topology within a voxel.  This table
// is of the form Table[256][41], but is stored as Lookup[10496].
// HLSL: StructuredBuffer<uint> lookup;
//
buffer lookup { uint data[]; } lookupSB;

// XSIZE-by-YSIZE-by-ZSIZE image stored in lexicographical order
// HLSL: StructuredBuffer<float> image;
//
buffer image { float data[]; } imageSB;

in Vertex
{
    uint index;
    uint configuration;
} inVertex[];

layout(location = 0) out vec3 tcoord;

layout(points) in;
layout(triangle_strip, max_vertices = 15) out;
void main()
{
    uint x = inVertex[0].index % XBOUND;
    uint q = inVertex[0].index / XBOUND;
    uint y = q % YBOUND;
    uint z = q / YBOUND;
    vec3 origin = vec3(x, y, z);

    uint i000 = inVertex[0].index;
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

    uint j = 41*inVertex[0].configuration;
    uint numVertices = lookupSB.data[j++];
    uint numTriangles = lookupSB.data[j++];

    // The position[] array must be initialized to avoid
    //    error X4580: emitting a system-interpreted value which is not
    //    written in every execution path of the shader
    // This error points to the stream.Append(output) call, so the HLSL
    // compiler cannot deduce when we are using only valid values in
    // position[].  This should be a warning, not an error.
    vec3 position[12] =
    {
        vec3(0,0,0), vec3(0,0,0), vec3(0,0,0), vec3(0,0,0),
        vec3(0,0,0), vec3(0,0,0), vec3(0,0,0), vec3(0,0,0),
        vec3(0,0,0), vec3(0,0,0), vec3(0,0,0), vec3(0,0,0)
    };

    uint i;
    for (i = 0; i < numVertices; ++i)
    {
        uint j0 = lookupSB.data[j++];
        uint j1 = lookupSB.data[j++];
        vec3 offset = (F[j0]*corner[j1] - F[j1]*corner[j0])/(F[j0] - F[j1]);
        position[i] = delta*(origin + offset);
    }
    j += 2*(12 - numVertices);  // Jump to the start of the triangle indices.

    for (i = 0; i < numTriangles; ++i)
    {
        for (uint k = 0; k < 3; ++k)
        {
            uint v = lookupSB.data[j++];
#if GTE_USE_MAT_VEC
            gl_Position = pvwMatrix * vec4(position[v], 1.0f);
#else
            gl_Position = vec4(position[v], 1.0f) * pvwMatrix;
#endif
            tcoord = 0.5f*position[v];
            EmitVertex();
        }
        EndPrimitive();
    }
}
