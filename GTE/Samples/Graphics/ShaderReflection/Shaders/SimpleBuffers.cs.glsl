// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

buffer InputBuffer
{
    float inputBuffer[];
};

buffer OutputBuffer
{
    vec4 outputBuffer[];
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec3 t = ivec3(gl_GlobalInvocationID.xyz);
    vec4 value = inputBuffer[0] * vec4(0.25f, 0.5f, 0.75f, 1.0f);
    outputBuffer[0] = value;
}
