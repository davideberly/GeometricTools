// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_ZERO_X_FACE
layout(r32f) uniform writeonly image3D image;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(image);
    imageStore(image, ivec3(0, c.y, c.z), vec4(0.0f));
    imageStore(image, ivec3(dim.x - 1, c.y, c.z), vec4(0.0f));
}
#endif

#if USE_ZERO_Y_FACE
layout(r32f) uniform writeonly image3D image;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(image);
    imageStore(image, ivec3(c.x, 0, c.z), vec4(0.0f));
    imageStore(image, ivec3(c.x, dim.y - 1, c.z), vec4(0.0f));
}
#endif

#if USE_ZERO_Z_FACE
layout(r32f) uniform writeonly image3D image;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(image);
    imageStore(image, ivec3(c.x, c.y, 0), vec4(0.0f));
    imageStore(image, ivec3(c.x, c.y, dim.z - 1), vec4(0.0f));
}
#endif
