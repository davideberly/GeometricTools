// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(r32f) uniform writeonly image3D poisson;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    imageStore(poisson, c, vec4(0.0f, 0.0f, 0.0f, 0.0f));
}
