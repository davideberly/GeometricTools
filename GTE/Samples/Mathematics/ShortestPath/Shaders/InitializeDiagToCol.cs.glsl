// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(rgba32f) uniform readonly image2D weights;
layout(rg32i) restrict uniform iimage2D previous;
layout(r32f) uniform writeonly image2D sum;

layout (local_size_x = ISIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int d = int(gl_GlobalInvocationID.x);
    imageStore(previous, ivec2(0, d), ivec4(0, d - 1, 0, 0));
    float w = imageLoad(weights, ivec2(0, d)).z;
    imageStore(sum, ivec2(d, d), vec4(w, 0.0f, 0.0f, 0.0f));
}
