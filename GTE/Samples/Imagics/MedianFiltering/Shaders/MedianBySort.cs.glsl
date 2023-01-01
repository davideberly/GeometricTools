// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(r32f) uniform readonly image2D inImage;
layout(r32f) uniform writeonly image2D outImage;

#define SIZE (2*RADIUS+1)
#define NUM_DATA (SIZE*SIZE)

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);

    // Load the neighborhood of the pixel.
    float data[NUM_DATA];
    int i = 0;
    ivec2 offset;
    for (offset.y = -RADIUS; offset.y <= RADIUS; ++offset.y)
    {
        for (offset.x = -RADIUS; offset.x <= RADIUS; ++offset.x)
        {
            data[i] = imageLoad(inImage, dt + offset).x;
            ++i;
        }
    }

    // Use an insertion sort to locate the median value.
    for (int i0 = 1; i0 < NUM_DATA; ++i0)
    {
        float value = data[i0];
        int i1;
        for (i1 = i0; i1 > 0; --i1)
        {
            if (value < data[i1 - 1])
            {
                data[i1] = data[i1 - 1];
            }
            else
            {
                break;
            }
        }
        data[i1] = value;
    }

    imageStore(outImage, dt, vec4(data[NUM_DATA / 2], 0.0f, 0.0f, 0.0f));
}
