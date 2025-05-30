// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

// The 3D image is stored as a tiled 2D texture, in which case the selection
// of neighboring pixels is handled by using a precomputed offset texture.

layout(r32f) uniform readonly image2D inImage;
layout(rg32i) uniform readonly iimage2D inOffset;
layout(r32f) uniform writeonly image2D outImage;

layout(local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);
    ivec2 offset = imageLoad(inOffset, t).rg;
    float result = imageLoad(inImage, t + offset).r;
    imageStore(outImage, t, vec4(result, 0, 0, 0));
}

