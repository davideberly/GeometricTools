// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform ControlPoints
{
    vec4 control[4];
};

layout(rgba32f) uniform writeonly image2D positions;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);

    ivec2 dims = imageSize(positions);

    vec2 uv = vec2(t) / vec2(dims);
    vec2 omuv = 1.0f - uv;
    vec2 uv2 = uv * uv;
    vec2 omuv2 = omuv * omuv;
    vec2 uvpoly[4] = { omuv2 * omuv, 3.0f * omuv2 * uv, 3.0f * omuv * uv2, uv * uv2 };

    float product[4];
    int i, j;

    for (i = 0; i < 4; ++i)
    {
        product[i] = 0.0f;
        for (j = 0; j < 4; ++j)
        {
            product[i] += uvpoly[j].y * control[i][j];
        }
    }

    float height = 0.0f;
    for (i = 0; i < 4; ++i)
    {
        height += uvpoly[i].x * product[i];
    }

    if (height > 0.0f)
    {
        imageStore(positions, t, vec4(vec2(t), height, 1.0f));
    }
    else
    {
        imageStore(positions, t, vec4(vec2(t), 0.0f, 0.0f));
    }
}
