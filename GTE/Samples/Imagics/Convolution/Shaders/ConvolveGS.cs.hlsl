// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Weights
{
    float weight[2 * RADIUS + 1][2 * RADIUS + 1];
};

Texture2D<float4> inImage;
RWTexture2D<float4> outImage;

// The test image is 1024x768.  The maximum number of bytes for
// 'samples' is 32768, so
//   (NUM_X_THREADS+2*RADIUS)*(NUM_Y_THREADS+2*RADIUS)
//     <= 32768/sizeof(float4) = 2048
// The application chooses NUM_X_THREADS = NUM_Y_THREADS = 16 and RADIUS = 8
// which uses 16384 bytes of group-shared memory.
groupshared float4 samples[NUM_Y_THREADS + 2 * RADIUS][NUM_X_THREADS + 2 * RADIUS];

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID, int2 gt : SV_GroupThreadID)
{
    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.y + RADIUS][gt.x + RADIUS] = inImage[dt];
    if (gt.y >= RADIUS)
    {
        if (gt.y < NUM_Y_THREADS - RADIUS)
        {
            if (gt.x >= RADIUS)
            {
                if (gt.x < NUM_X_THREADS - RADIUS)
                {
                    // 00: No extra inputs to load.
                }
                else
                {
                    // +0
                    samples[gt.y + RADIUS][gt.x + 2 * RADIUS] = inImage[dt + int2(+RADIUS, 0)];
                }
            }
            else
            {
                // -0
                samples[gt.y + RADIUS][gt.x] = inImage[dt + int2(-RADIUS, 0)];
            }
        }
        else
        {
            if (gt.x >= RADIUS)
            {
                if (gt.x < NUM_X_THREADS - RADIUS)
                {
                    // 0+
                    samples[gt.y + 2 * RADIUS][gt.x + RADIUS] = inImage[dt + int2(0, +RADIUS)];
                }
                else
                {
                    // ++
                    samples[gt.y + 2 * RADIUS][gt.x + 2 * RADIUS] = inImage[dt + int2(+RADIUS, +RADIUS)];
                    samples[gt.y + 2 * RADIUS][gt.x + RADIUS] = inImage[dt + int2(0, +RADIUS)];
                    samples[gt.y + RADIUS][gt.x + 2 * RADIUS] = inImage[dt + int2(+RADIUS, 0)];
                }
            }
            else
            {
                // -+
                samples[gt.y + 2 * RADIUS][gt.x] = inImage[dt + int2(-RADIUS, +RADIUS)];
                samples[gt.y + 2 * RADIUS][gt.x + RADIUS] = inImage[dt + int2(0, +RADIUS)];
                samples[gt.y + RADIUS][gt.x] = inImage[dt + int2(-RADIUS, 0)];
            }
        }
    }
    else
    {
        if (gt.x >= RADIUS)
        {
            if (gt.x < NUM_X_THREADS - RADIUS)
            {
                // 0-
                samples[gt.y][gt.x + RADIUS] = inImage[dt + int2(0, -RADIUS)];
            }
            else
            {
                // +-
                samples[gt.y][gt.x + 2 * RADIUS] = inImage[dt + int2(+RADIUS, -RADIUS)];
                samples[gt.y][gt.x + RADIUS] = inImage[dt + int2(0, -RADIUS)];
                samples[gt.y + RADIUS][gt.x + 2 * RADIUS] = inImage[dt + int2(+RADIUS, 0)];
            }
        }
        else
        {
            // --
            samples[gt.y][gt.x] = inImage[dt + int2(-RADIUS, -RADIUS)];
            samples[gt.y][gt.x + RADIUS] = inImage[dt + int2(0, -RADIUS)];
            samples[gt.y + RADIUS][gt.x] = inImage[dt + int2(-RADIUS, 0)];
        }
    }

    GroupMemoryBarrierWithGroupSync();

    float4 result = 0.0f;
    for (int y = 0; y <= 2 * RADIUS; ++y)
    {
        for (int x = 0; x <= 2 * RADIUS; ++x)
        {
            result += weight[y][x] * samples[gt.y + y][gt.x + x];
        }
    }
    outImage[dt] = result;
}
