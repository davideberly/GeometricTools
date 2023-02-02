// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_COPY_X_EDGE
layout(rgba32f) uniform readonly image2D state;
layout(r32f) uniform writeonly image1D xMin;
layout(r32f) uniform writeonly image1D xMax;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(state);
    float xMinValue = imageLoad(state, ivec2(1, c.y)).y;
    float xMaxValue = imageLoad(state, ivec2(dim.x - 2, c.y)).y;
    imageStore(xMin, c.y, vec4(xMinValue, 0.0f, 0.0f, 0.0f));
    imageStore(xMax, c.y, vec4(xMaxValue, 0.0f, 0.0f, 0.0f));
}
#endif

#if USE_WRITE_X_EDGE
layout(r32f) uniform readonly image1D xMin;
layout(r32f) uniform readonly image1D xMax;
layout(rgba32f) uniform writeonly image2D state;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(state);
    float xMinValue = imageLoad(xMin, c.y).x;
    float xMaxValue = imageLoad(xMax, c.y).x;
    imageStore(state, ivec2(0, c.y), vec4(0.0f, xMinValue, 0.0f, 0.0f));
    imageStore(state, ivec2(dim.x - 1, c.y), vec4(0.0f, xMaxValue, 0.0f, 0.0f));
}
#endif

#if USE_COPY_Y_EDGE
layout(rgba32f) uniform readonly image2D state;
layout(r32f) uniform writeonly image1D yMin;
layout(r32f) uniform writeonly image1D yMax;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(state);
    float yMinValue = imageLoad(state, ivec2(c.x, 1)).x;
    float yMaxValue = imageLoad(state, ivec2(c.x, dim.y - 2)).x;
    imageStore(yMin, c.x, vec4(yMinValue, 0.0f, 0.0f, 0.0f));
    imageStore(yMax, c.x, vec4(yMaxValue, 0.0f, 0.0f, 0.0f));
}
#endif

#if USE_WRITE_Y_EDGE
layout(r32f) uniform readonly image1D yMin;
layout(r32f) uniform readonly image1D yMax;
layout(rgba32f) uniform writeonly image2D state;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(state);
    float yMinValue = imageLoad(yMin, c.x).x;
    float yMaxValue = imageLoad(yMax, c.x).x;
    imageStore(state, ivec2(c.x, 0), vec4(yMinValue, 0.0f, 0.0f, 0.0f));
    imageStore(state, ivec2(c.x, dim.y - 1), vec4(yMaxValue, 0.0f, 0.0f, 0.0f));
}
#endif;
