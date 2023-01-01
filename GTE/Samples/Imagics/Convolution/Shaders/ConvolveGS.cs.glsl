// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#define WSIZE (2 * RADIUS + 1)

uniform Weights
{
    // (2R+1)-by-(2R+1) mask stored in row-major order.
    float weight[WSIZE * WSIZE];
};

layout(rgba32f) uniform readonly image2D inImage;
layout(rgba32f) uniform writeonly image2D outImage;

// The test image is 1024x768.  The maximum number of bytes for
// 'samples' is 32768, so
//   (NUM_X_THREADS+2*RADIUS)*(NUM_Y_THREADS+2*RADIUS)
//     <= 32768/sizeof(float4) = 2048
// The application chooses NUM_X_THREADS = NUM_Y_THREADS = 16 and RADIUS = 8
// which uses 16384 bytes of group-shared memory.
shared vec4 samples[NUM_Y_THREADS + 2 * RADIUS][NUM_X_THREADS + 2 * RADIUS];

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    ivec2 gt = ivec2(gl_LocalInvocationID.xy);

    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.y + RADIUS][gt.x + RADIUS] = imageLoad(inImage, dt);
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
                    samples[gt.y + RADIUS][gt.x + 2 * RADIUS] =
                        imageLoad(inImage, dt + ivec2(+RADIUS, 0));
                }
            }
            else
            {
                // -0
                samples[gt.y + RADIUS][gt.x] =
                    imageLoad(inImage, dt + ivec2(-RADIUS, 0));
            }
        }
        else
        {
            if (gt.x >= RADIUS)
            {
                if (gt.x < NUM_X_THREADS - RADIUS)
                {
                    // 0+
                    samples[gt.y + 2 * RADIUS][gt.x + RADIUS] =
                        imageLoad(inImage, dt + ivec2(0, +RADIUS));
                }
                else
                {
                    // ++
                    samples[gt.y + 2 * RADIUS][gt.x + 2 * RADIUS] =
                        imageLoad(inImage, dt + ivec2(+RADIUS, +RADIUS));
                    samples[gt.y + 2 * RADIUS][gt.x + RADIUS] =
                        imageLoad(inImage, dt + ivec2(0, +RADIUS));
                    samples[gt.y + RADIUS][gt.x + 2 * RADIUS] =
                        imageLoad(inImage, dt + ivec2(+RADIUS, 0));
                }
            }
            else
            {
                // -+
                samples[gt.y + 2 * RADIUS][gt.x] =
                    imageLoad(inImage, dt + ivec2(-RADIUS, +RADIUS));
                samples[gt.y + 2 * RADIUS][gt.x + RADIUS] =
                    imageLoad(inImage, dt + ivec2(0, +RADIUS));
                samples[gt.y + RADIUS][gt.x] =
                    imageLoad(inImage, dt + ivec2(-RADIUS, 0));
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
                samples[gt.y][gt.x + RADIUS] =
                    imageLoad(inImage, dt + ivec2(0, -RADIUS));
            }
            else
            {
                // +-
                samples[gt.y][gt.x + 2 * RADIUS] =
                    imageLoad(inImage, dt + ivec2(+RADIUS, -RADIUS));
                samples[gt.y][gt.x + RADIUS] =
                    imageLoad(inImage, dt + ivec2(0, -RADIUS));
                samples[gt.y + RADIUS][gt.x + 2 * RADIUS] =
                    imageLoad(inImage, dt + ivec2(+RADIUS, 0));
            }
        }
        else
        {
            // --
            samples[gt.y][gt.x] =
                imageLoad(inImage, dt + ivec2(-RADIUS, -RADIUS));
            samples[gt.y][gt.x + RADIUS] =
                imageLoad(inImage, dt + ivec2(0, -RADIUS));
            samples[gt.y + RADIUS][gt.x] =
                imageLoad(inImage, dt + ivec2(-RADIUS, 0));
        }
    }

    groupMemoryBarrier();

    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int y = 0; y <= 2 * RADIUS; ++y)
    {
        for (int x = 0; x <= 2 * RADIUS; ++x)
        {
            result += weight[x + WSIZE * y] * samples[gt.y + y][gt.x + x];
        }
    }
    imageStore(outImage, dt, result);
}
