// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(rgba32f) uniform readonly image2D inImage;
layout(rgba32f) uniform writeonly image2D outImage;

const float weight[3][3] =
{
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
    { 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f },
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f }
};

const ivec2 offset[3][3] =
{
    { ivec2(-1, -1), ivec2(0, -1), ivec2(+1, -1) },
    { ivec2(-1,  0), ivec2(0,  0), ivec2(+1,  0) },
    { ivec2(-1, +1), ivec2(0, +1), ivec2(+1, +1) }
};

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 3; ++c)
        {
            result += weight[r][c] * imageLoad(inImage, t + offset[r][c]);
        }
    }
    imageStore(outImage, t, vec4(result.rgb, 1.0f));
}
