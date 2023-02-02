// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_ZERO_X_EDGE
layout(r32f) uniform writeonly image2D image;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(image);
    imageStore(image, ivec2(0, c.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    imageStore(image, ivec2(dim.x - 1, c.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}
#endif

#if USE_ZERO_Y_EDGE
layout(r32f) uniform writeonly image2D image;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(image);
    imageStore(image, ivec2(c.x, 0), vec4(0.0f, 0.0f, 0.0f, 0.0f));
    imageStore(image, ivec2(c.x, dim.y - 1), vec4(0.0f, 0.0f, 0.0f, 0.0f));
}
#endif;
