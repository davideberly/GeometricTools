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

layout(r32f) uniform readonly image2D random;
layout(rgba32f) uniform writeonly image2D weights;

// Compute the height using a bicubic polynomial that does not have the convex
// hull property.  Clamp below to zero but scale down rather than saturate to
// avoid flat spots in the graph.
float GetHeight(vec2 uv)
{
    vec2 omuv = 1.0f - uv;
    vec2 uv2 = uv * uv;
    vec2 omuv2 = omuv * omuv;
    vec2 uvpoly[4] = { omuv2*omuv, 3.0f*omuv2*uv, 3.0f*omuv*uv2, uv*uv2 };

    float product[4];
    for (int i = 0; i < 4; ++i)
    {
        product[i] = 0.0f;
        for (int j = 0; j < 4; ++j)
        {
            product[i] += uvpoly[j].y * control[i][j];
        }
    }

    float height = 0.0f;
    for (int k = 0; k < 4; ++k)
    {
        height += uvpoly[k].x * product[k];
    }
    height = max(0.5f*height, 0.0f);
    return height;
}

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 dt = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(weights);
    vec2 fdims = vec2(dims);

    float height00 = GetHeight(vec2(dt) / fdims) + imageLoad(random, dt).x;
    ivec2 dt10 = dt + ivec2(1, 0);
    float height10 = GetHeight(vec2(dt10) / fdims) + imageLoad(random, dt10).x;
    ivec2 dt01 = dt + ivec2(0, 1);
    float height01 = GetHeight(vec2(dt01) / fdims) + imageLoad(random, dt01).x;
    ivec2 dt11 = dt + ivec2(1, 1);
    float height11 = GetHeight(vec2(dt11) / fdims) + imageLoad(random, dt11).x;
    float weight1 = 0.5f*(height00 + height10);
    float weight2 = 0.5f*(height00 + height01);
    float weight3 = 0.70710678f*(height00 + height11);
    imageStore(weights, dt, vec4(height00, weight1, weight2, weight3));
}
