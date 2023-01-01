// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_ZERO_X_FACE
RWTexture3D<float> image;

[numthreads(1, NUM_Y_THREADS, NUM_Z_THREADS)]
void WriteXFace(uint3 c : SV_DispatchThreadID)
{
    uint3 dim;
    image.GetDimensions(dim.x, dim.y, dim.z);
    image[uint3(0, c.y, c.z)] = 0.0f;
    image[uint3(dim.x - 1, c.y, c.z)] = 0.0f;
}
#endif

#if USE_ZERO_Y_FACE
RWTexture3D<float> image;

[numthreads(NUM_X_THREADS, 1, NUM_Z_THREADS)]
void WriteYFace(uint3 c : SV_DispatchThreadID)
{
    uint3 dim;
    image.GetDimensions(dim.x, dim.y, dim.z);
    image[uint3(c.x, 0, c.z)] = 0.0f;
    image[uint3(c.x, dim.y - 1, c.z)] = 0.0f;
}
#endif

#if USE_ZERO_Z_FACE
RWTexture3D<float> image;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void WriteZFace(uint3 c : SV_DispatchThreadID)
{
    uint3 dim;
    image.GetDimensions(dim.x, dim.y, dim.z);
    image[uint3(c.x, c.y, 0)] = 0.0f;
    image[uint3(c.x, c.y, dim.z - 1)] = 0.0f;
}
#endif
