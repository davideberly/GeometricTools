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

#define NUM_X_THREADS 1024
shared vec4 samples[NUM_X_THREADS + 2 * RADIUS];

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
        samples[gt.x + 2 * RADIUS] = imageLoad(inImage, dt + ivec2(RADIUS, 0));
    }
    groupMemoryBarrier();

    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int x = 0; x <= 2*RADIUS; ++x)
    {
        result += weight[x] * samples[gt.x + x];
    }
    imageStore(outImage, dt, result);
}
