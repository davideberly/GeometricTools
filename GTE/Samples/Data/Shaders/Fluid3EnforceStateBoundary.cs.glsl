// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#if USE_COPY_X_FACE
layout(rgba32f) uniform readonly image3D state;
layout(rg32f) uniform writeonly image2D xMin;
layout(rg32f) uniform writeonly image2D xMax;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 xMinValue = imageLoad(state, ivec3(1, c.y, c.z)).yz;
    vec2 xMaxValue = imageLoad(state, ivec3(dim.x - 2, c.y, c.z)).yz;
    imageStore(xMin, c.yz, vec4(xMinValue, 0.0f, 0.0f));
    imageStore(xMax, c.yz, vec4(xMaxValue, 0.0f, 0.0f));
}
#endif

#if USE_WRITE_X_FACE
layout(rg32f) uniform readonly image2D xMin;
layout(rg32f) uniform readonly image2D xMax;
layout(rgba32f) uniform writeonly image3D state;

layout (local_size_x = 1, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 xMinValue = imageLoad(xMin, c.yz).xy;
    vec2 xMaxValue = imageLoad(xMax, c.yz).xy;
    imageStore(state, ivec3(0, c.y, c.z), vec4(0.0f, xMinValue.x, xMinValue.y, 0.0f));
    imageStore(state, ivec3(dim.x - 1, c.y, c.z), vec4(0.0f, xMaxValue.x, xMaxValue.y, 0.0f));
}
#endif

#if USE_COPY_Y_FACE
layout(rgba32f) uniform readonly image3D state;
layout(rg32f) uniform writeonly image2D yMin;
layout(rg32f) uniform writeonly image2D yMax;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 yMinValue = imageLoad(state, ivec3(c.x, 1, c.z)).xz;
    vec2 yMaxValue = imageLoad(state, ivec3(c.x, dim.y - 2, c.z)).xz;
    imageStore(yMin, c.xz, vec4(yMinValue, 0.0f, 0.0f));
    imageStore(yMax, c.xz, vec4(yMaxValue, 0.0f, 0.0f));
}
#endif

#if USE_WRITE_Y_FACE
layout(rg32f) uniform readonly image2D yMin;
layout(rg32f) uniform readonly image2D yMax;
layout(rgba32f) uniform writeonly image3D state;

layout (local_size_x = NUM_X_THREADS, local_size_y = 1, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 yMinValue = imageLoad(yMin, c.xz).xy;
    vec2 yMaxValue = imageLoad(yMax, c.xz).xy;
    imageStore(state, ivec3(c.x, 0, c.z), vec4(yMinValue.x, 0.0f, yMinValue.y, 0.0f));
    imageStore(state, ivec3(c.x, dim.y - 1, c.z), vec4(yMaxValue.x, 0.0f, yMaxValue.y, 0.0f));
}
#endif

#if USE_COPY_Z_FACE
layout(rgba32f) uniform readonly image3D state;
layout(rg32f) uniform writeonly image2D zMin;
layout(rg32f) uniform writeonly image2D zMax;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 zMinValue = imageLoad(state, ivec3(c.x, c.y, 1)).xy;
    vec2 zMaxValue = imageLoad(state, ivec3(c.x, c.y, dim.z - 2)).xy;
    imageStore(zMin, c.xy, vec4(zMinValue, 0.0f, 0.0f));
    imageStore(zMax, c.xy, vec4(zMaxValue, 0.0f, 0.0f));
}
#endif

#if USE_WRITE_Z_FACE
layout(rg32f) uniform readonly image2D zMin;
layout(rg32f) uniform readonly image2D zMax;
layout(rgba32f) uniform writeonly image3D state;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(state);
    vec2 zMinValue = imageLoad(zMin, c.xy).xy;
    vec2 zMaxValue = imageLoad(zMax, c.xy).xy;
    imageStore(state, ivec3(c.x, c.y, 0), vec4(zMinValue.x, zMinValue.y, 0.0f, 0.0f));
    imageStore(state, ivec3(c.x, c.y, dim.z - 1), vec4(zMaxValue.x, zMaxValue.y, 0.0f, 0.0f));
}
#endif
