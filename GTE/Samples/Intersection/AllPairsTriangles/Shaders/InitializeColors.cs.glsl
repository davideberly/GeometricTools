// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

buffer color0 { uint data[]; } color0SB;
buffer color1 { uint data[]; } color1SB;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    for (int j = 0; j < 3; ++j)
    {
        color0SB.data[3*dt.x + j] = 0;
        color1SB.data[3*dt.y + j] = 1;
    }
}
