// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Weights
{
    float weight[2 * RADIUS + 1];
};

Texture2D<float4> inImage;
RWTexture2D<float4> outImage;

// See the comments in ConvolveSeparableHGS2.hlsl.

#define NUM_Y_THREADS 192
groupshared float4 samples[NUM_Y_THREADS + 2*RADIUS];

// numXGroups = 1024, numYGroups = 4
[numthreads(1, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID, int2 gt : SV_GroupThreadID)
{
    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.y + RADIUS] = inImage[dt];
    if (gt.y < RADIUS)
    {
        samples[gt.y] = inImage[dt - int2(0, RADIUS)];
    }
    else if (gt.y >= NUM_Y_THREADS - RADIUS)
    {
        samples[gt.y + 2 * RADIUS] = inImage[dt + int2(0, RADIUS)];
    }
    GroupMemoryBarrierWithGroupSync();

    float4 result = 0.0f;
    for (int y = 0; y <= 2*RADIUS; ++y)
    {
        result += weight[y] * samples[gt.y + y];
    }
    outImage[dt] = result;
}
