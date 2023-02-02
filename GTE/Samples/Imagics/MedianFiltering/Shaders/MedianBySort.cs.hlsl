// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float> inImage;
RWTexture2D<float> outImage;

#define SIZE (2*RADIUS+1)
#define NUM_DATA (SIZE*SIZE)

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    // Load the neighborhood of the pixel.
    float data[NUM_DATA];
    int i = 0;
    int2 offset;
    [unroll]
    for (offset.y = -RADIUS; offset.y <= RADIUS; ++offset.y)
    {
        [unroll]
        for (offset.x = -RADIUS; offset.x <= RADIUS; ++offset.x)
        {
            data[i] = inImage[dt + offset];
            ++i;
        }
    }

    // Use an insertion sort to locate the median value.  NOTE:  If you add
    // [unroll] directives to these loops using the Microsoft (R) Direct3D
    // Shader compiler 6.3.9600.16384, the median output is clearly different
    // visually.  Perhaps the compiled code is incorrect.
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

    outImage[dt] = data[NUM_DATA / 2];
}
