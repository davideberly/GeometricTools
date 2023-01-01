// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#include "MedianShared.cs.hlsli"

Texture2D<float> inImage;
RWTexture2D<float> outImage;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    // Load the neighborhood of the pixel.  The use of float4 allows
    // vectorization in minmaxN for speed.
    float4 e[NUM_ELEMENTS];  // 28 slots, we use the first 25
    LoadNeighbors(inImage, dt, e);

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
    outImage[dt] = e[0].y;
}
