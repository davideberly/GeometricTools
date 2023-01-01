// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_ZERO_X_EDGE
RWTexture2D<float> image;

[numthreads(1, NUM_Y_THREADS, 1)]
void WriteXEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    image.GetDimensions(dim.x, dim.y);
    image[uint2(0, c.y)] = 0.0f;
    image[uint2(dim.x - 1, c.y)] = 0.0f;
}
#endif

#if USE_ZERO_Y_EDGE
RWTexture2D<float> image;

[numthreads(NUM_X_THREADS, 1, 1)]
void WriteYEdge(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    image.GetDimensions(dim.x, dim.y);
    image[uint2(c.x, 0)] = 0.0f;
    image[uint2(c.x, dim.y - 1)] = 0.0f;
}
#endif
