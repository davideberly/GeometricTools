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

// This is an example of how one might handle loading to group-shared memory
// when the entire row of an image is too large to fit, requiring instead
// more thread groups each with a smaller number of threads.  The test image
// is 1024x768.  The number of x-threads is chosen to be 256.  The groups
// have x-dispatch-thread ids {0..255}, {256..511}, {512, 767}, {768, 1023}.
// For a single group, the convolution mask extends outside these intervals.
// We need to load image values outside the intervals.  Once choice is to have
// the threads for the min/max ids load the extra values to group-shared
// memory.  For example,
//   x-dispatch-thread 256 loads input[(256-RADIUS,y)] through input[(256,y)]
//   x-dispatch-thread 511 loads input[(511,y)] through input[(511+RADIUS,y)]
// For large RADIUS, these 2 threads in the group can stall the others
// (due to the group synchronization call) trying to load a lot of data.
// Alternatively, you can distribute the boundary loads across the threads.
// For example,
//   x-dispatch-thread 256 loads input[(256-RADIUS,y)] and input[(256,y)]
//   x-dispatch-thread 257 loads input[(257-RADIUS,y)] and input[(257,y)]
// and so on until the left-boundary values are loaded.  In this example I
// have chosen this distributed loading.
//
// NOTE:  You need to be careful how you write to group-shared memory.  The
// documentation on store_raw indicates that any write to out-of-bounds for
// your specific 'groupshared' array will cause the entire contents of that
// array to be undefined.

#define NUM_X_THREADS 256
groupshared float4 samples[NUM_X_THREADS + 2*RADIUS];

// numXGroups = 4, numYGroups = 768
[numthreads(NUM_X_THREADS, 1, 1)]
void CSMain(int2 dt : SV_DispatchThreadID, int2 gt : SV_GroupThreadID)
{
    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.x + RADIUS] = inImage[dt];
    if (gt.x < RADIUS)
    {
        samples[gt.x] = inImage[dt - int2(RADIUS, 0)];
    }
    else if (gt.x >= NUM_X_THREADS - RADIUS)
    {
        samples[gt.x + 2*RADIUS] = inImage[dt + int2(RADIUS, 0)];
    }
    GroupMemoryBarrierWithGroupSync();

    float4 result = 0.0f;
    for (int x = 0; x <= 2*RADIUS; ++x)
    {
        result += weight[x] * samples[gt.x + x];
    }
    outImage[dt] = result;
}
