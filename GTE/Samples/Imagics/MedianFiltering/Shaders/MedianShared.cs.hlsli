// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#define SIZE (2*RADIUS + 1)
#define NUM_NEIGHBORS (SIZE * SIZE)
#define NUM_ELEMENTS (1 + (NUM_NEIGHBORS - 1)/4)

void LoadNeighbors(in Texture2D<float> input, in int2 dt, out float4 e[NUM_ELEMENTS])
{
    uint i = 0;
    int2 offset;
    [unroll]
    for (offset.y = -RADIUS; offset.y <= RADIUS; ++offset.y)
    {
        [unroll]
        for (offset.x = -RADIUS; offset.x <= RADIUS; ++offset.x)
        {
            e[i / 4][i % 4] = input[dt + offset];
            ++i;
        }
    }
}

void minmax(inout float u, inout float v)
{
    float save = u;
    u = min(save, v);
    v = max(save, v);
}

void minmax(inout float2 u, inout float2 v)
{
    float2 save = u;
    u = min(save, v);
    v = max(save, v);
}

void minmax(inout float4 u, inout float4 v)
{
    float4 save = u;
    u = min(save, v);
    v = max(save, v);
}

void minmax3(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0].x, e[0].y);      // min in e0.xz, max in e0.yz
    minmax(e[0].x, e[0].z);      // min in e0.x, max in e0.yz
    minmax(e[0].y, e[0].z);      // min in e0.x, max in e0.z
}

void minmax4(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e0.zw
    minmax(e[0].xz, e[0].yw);    // min in e0.x, max in e0.w
}

void minmax5(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0].xy, e[0].zw);    // min in {e0.xy, e1.x}, max in {e0.zw, e1.x}
    minmax(e[0].xz, e[0].yw);    // min in {e0.x, e1.x}, max in {e0.w, e1.x}
    minmax(e[0].x, e[1].x);      // min in e0.x, max in {e0.w, e1.x}
    minmax(e[0].w, e[1].x);      // min in e0.x, max in e1.x
}

void minmax6(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0].xy, e[0].zw);    // min in {e0.xy, e1.xy}, max in {e0.zw, e1.xy}
    minmax(e[0].xz, e[0].yw);    // min in {e0.x, e1.xy}, max in {e0.w, e1.xy}
    minmax(e[1].x, e[1].y);      // min in {e0.x, e1.x}, max in {e0.w, e1.y}
    minmax(e[0].xw, e[1].xy);    // min in e0.x, max in e1.y
}

void minmax8(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0], e[1]);          // min in e0, max in e1
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e1
    minmax(e[0].x, e[0].y);      // min in e0.x, max in e1
    minmax(e[1].xy, e[1].zw);    // min in e0.x, max in e1.zw
    minmax(e[1].z, e[1].w);      // min in e0.x, max in e1.w
}

void minmax12(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0], e[1]);          // min in {e0, e2}, max in {e1, e2}
    minmax(e[0], e[2]);          // min in e0, max in {e1, e2}
    minmax(e[1], e[2]);          // min in e0, max in e2
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e2
    minmax(e[0].xz, e[0].yw);    // min in e0.x, max in e2
    minmax(e[2].xy, e[2].zw);    // min in e0.x, max in e2.xy
    minmax(e[2].xz, e[2].yw);    // min in e0.x, max in e2.y
}

void minmax16(inout float4 e[NUM_ELEMENTS])
{
    minmax(e[0], e[1]);          // min in {e0, e2, e3}, max in {e1, e2, e3}
    minmax(e[2], e[3]);          // min in {e0, e2}, max in {e1, e3}
    minmax(e[0], e[2]);          // min in e0, max in {e1, e3}
    minmax(e[1], e[3]);          // min in e0, max in e3
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e3
    minmax(e[0].xz, e[0].yw);    // min in e0.x, max in e3
    minmax(e[3].xy, e[3].zw);    // min in e0.x, max in e3.xy
    minmax(e[3].xz, e[3].yw);    // min in e0.x, max in e3.y
}
