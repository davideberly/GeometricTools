// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Weights
{
    float weight[2 * RADIUS + 1];
};

layout(rgba32f) uniform readonly image2D inImage;
layout(rgba32f) uniform writeonly image2D outImage;

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

#define NUM_X_THREADS 256
shared vec4 samples[NUM_X_THREADS + 2 * RADIUS];

// numXGroups = 4, numYGroups = 768
layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    ivec2 gt = ivec2(gl_LocalInvocationID.xy);

    // Load the texels from the input texture, store them in group-shared
    // memory, and have all threads in the group wait until all texels
    // are loaded.
    samples[gt.x + RADIUS] = imageLoad(inImage, dt);
    if (gt.x < RADIUS)
    {
        samples[gt.x] = imageLoad(inImage, dt - ivec2(RADIUS, 0));
    }
    else if (gt.x >= NUM_X_THREADS - RADIUS)
    {
        samples[gt.x + 2*RADIUS] = imageLoad(inImage, dt + ivec2(RADIUS, 0));
    }
    groupMemoryBarrier();

    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int x = 0; x <= 2*RADIUS; ++x)
    {
        result += weight[x] * samples[gt.x + x];
    }
    imageStore(outImage, dt, result);
}
