// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

// Each shader is launched with numXGroups = numYGroups = numZGroups = 1.
// The 'sum' image is therefore limited to 1024x1024 (1024 partial sums).
// For 0 <= x < 2^{n-p} and 0 <= y < 2^{p-1},
//     S(2^p*x, 2^p*x + 2^{p-1} + y) =
//         S(2^p*x, 2^p*x + 2^{p-1} - 1) +
//         S(2^p*x + 2^{p-1}, 2^p*x + 2^{p-1} + y)

// The application must define LOGN and P.  For a specified LOGN, you need
// shaders for each P with 1 <= P <= LOGN.
//#define NUM_X_THREADS (1 << (LOGN-P))
//#define NUM_Y_THREADS (1 << (P-1))
//#define TWO_P (1 << P)
//#define TWO_PM1 (1 << (P-1))

layout(r32f) restrict uniform image2D sum;

// Initial values are on the diagonal of the square.  After all shaders
// are called, the results are in row 0 of the square.
layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);
    float input0 = imageLoad(sum, ivec2(TWO_P * t.x + TWO_PM1 - 1, TWO_P * t.x)).x;
    float input1 = imageLoad(sum, ivec2(TWO_P * t.x + TWO_PM1 + t.y, TWO_P * t.x + TWO_PM1)).x;
    float result = input0 + input1;
    imageStore(sum, ivec2(TWO_P * t.x + TWO_PM1 + t.y, TWO_P * t.x), vec4(result, 0.0f, 0.0f, 0.0f));
}
