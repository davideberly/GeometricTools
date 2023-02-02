// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(r32f) uniform readonly image2D inImage;
layout(r32f) uniform writeonly image2D outImage;

void LoadNeighbors(in ivec2 dt, out vec4 e[7])
{
    uint i = 0;
    ivec2 offset;
    for (offset.y = -2; offset.y <= 2; ++offset.y)
    {
        for (offset.x = -2; offset.x <= 2; ++offset.x)
        {
            e[i / 4][i % 4] = imageLoad(inImage, dt + offset).x;
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

void minmax(inout vec2 u, inout vec2 v)
{
    vec2 save = u;
    u = min(save, v);
    v = max(save, v);
}

void minmax(inout vec4 u, inout vec4 v)
{
    vec4 save = u;
    u = min(save, v);
    v = max(save, v);
}

void minmax3(inout vec4 e[7])
{
    minmax(e[0].x, e[0].y);      // min in e0.xz, max in e0.yz
    minmax(e[0].x, e[0].z);      // min in e0.x, max in e0.yz
    minmax(e[0].y, e[0].z);      // min in e0.x, max in e0.z
}

void minmax4(inout vec4 e[7])
{
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e0.zw
    minmax(e[0].xz, e[0].yw);    // min in e0.x, max in e0.w
}

void minmax5(inout vec4 e[7])
{
    minmax(e[0].xy, e[0].zw);    // min in {e0.xy, e1.x}, max in {e0.zw, e1.x}
    minmax(e[0].xz, e[0].yw);    // min in {e0.x, e1.x}, max in {e0.w, e1.x}
    minmax(e[0].x, e[1].x);      // min in e0.x, max in {e0.w, e1.x}
    minmax(e[0].w, e[1].x);      // min in e0.x, max in e1.x
}

void minmax6(inout vec4 e[7])
{
    minmax(e[0].xy, e[0].zw);    // min in {e0.xy, e1.xy}, max in {e0.zw, e1.xy}
    minmax(e[0].xz, e[0].yw);    // min in {e0.x, e1.xy}, max in {e0.w, e1.xy}
    minmax(e[1].x, e[1].y);      // min in {e0.x, e1.x}, max in {e0.w, e1.y}
    minmax(e[0].xw, e[1].xy);    // min in e0.x, max in e1.y
}

void minmax8(inout vec4 e[7])
{
    minmax(e[0], e[1]);          // min in e0, max in e1
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e1
    minmax(e[0].x, e[0].y);      // min in e0.x, max in e1
    minmax(e[1].xy, e[1].zw);    // min in e0.x, max in e1.zw
    minmax(e[1].z, e[1].w);      // min in e0.x, max in e1.w
}

void minmax12(inout vec4 e[7])
{
    minmax(e[0], e[1]);          // min in {e0, e2}, max in {e1, e2}
    minmax(e[0], e[2]);          // min in e0, max in {e1, e2}
    minmax(e[1], e[2]);          // min in e0, max in e2
    minmax(e[0].xy, e[0].zw);    // min in e0.xy, max in e2
    minmax(e[0].xz, e[0].yw);    // min in e0.x, max in e2
    minmax(e[2].xy, e[2].zw);    // min in e0.x, max in e2.xy
    minmax(e[2].xz, e[2].yw);    // min in e0.x, max in e2.y
}

void minmax16(inout vec4 e[7])
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

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);

    // Load the neighborhood of the pixel.  The use of vec4 allows
    // vectorization in minmaxN for speed.
    vec4 e[7];  // 28 slots, we use the first 25
    LoadNeighbors(dt, e);

    // Repeated removal of minima and maxima.
    minmax16(e);        // Discard min and max of v0..v15 (2n+1 = 25, 16 > n+2 = 14).
    e[0][0] = e[6][0];  // Copy v24 to v0 slot.
    e[3][3] = e[5][3];  // Copy v23 to v15 slot.
    minmax16(e);        // Discard min and max of v0..v15 (2n+1 = 23, 16 > n+2 = 13).
    e[0][0] = e[5][2];  // Copy v22 to v0 slot.
    e[3][3] = e[5][1];  // Copy v21 to v15 slot.
    minmax12(e);        // Discard min and max of v0..v11 (2n+1 = 21, n+2 = 12).
    e[0][0] = e[5][0];  // Copy v20 to v0 slot.
    e[2][3] = e[4][3];  // Copy v19 to v11 slot.
    minmax12(e);        // Discard min and max of v0..v11 (2n+1 = 19, 12 > n+2 = 11).
    e[0][0] = e[4][2];  // Copy v18 to v0 slot.
    e[2][3] = e[4][1];  // Copy v17 to v11 slot.
    minmax12(e);        // Discard min and max of v0..v11 (2n+1 = 17, 12 > n+2 = 10).
    e[0][0] = e[4][0];  // Copy v16 to v0 slot.
    e[2][3] = e[3][3];  // Copy v15 to v11 slot.
    minmax12(e);        // Discard min and max of v0..v11 (2n+1 = 15, 12 > n+2 = 9).
    e[0][0] = e[3][2];  // Copy v14 to v0 slot.
    e[2][3] = e[3][1];  // Copy v13 to v11 slot.
    minmax8(e);         // Discard min and max of v0..v7 (2n+1 = 13, n+2 = 8).
    e[0][0] = e[3][0];  // Copy v12 to v0 slot.
    e[1][3] = e[2][3];  // Copy v11 to v7 slot.
    minmax8(e);         // Discard min and max of v0..v7 (2n+1 = 11, 8 > n+2 = 7).
    e[0][0] = e[2][2];  // Copy v10 to v0 slot.
    e[1][3] = e[2][1];  // Copy v9 to v7 slot.
    minmax6(e);         // Discard min and max of v0..v5 (2n+1=9, n+2=6).
    e[0].x = e[2].x;    // Copy v8 to v0 slot.
    minmax5(e);         // Discard min and max of v0..v4 (2n+1=7, n+2=5).
    e[0].x = e[1].w;    // Copy v7 to v0 slot.
    minmax4(e);         // Discard min and max of v0..v3 (2n+1=5, n+2=4).
    e[0].x = e[1].z;    // Copy v6 to v0 slot.
    minmax3(e);         // Sort v0, v1, and v2.

    // Return the median v1.
    imageStore(outImage, dt, vec4(e[0].y, 0.0f, 0.0f, 0.0f));
}
