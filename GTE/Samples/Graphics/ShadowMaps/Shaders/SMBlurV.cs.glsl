// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

layout(rgba32f) uniform readonly image2D inImage;
layout(rgba32f) uniform writeonly image2D outImage;

const float weight[11] =
{
    1.48671961e-06f,
    0.000133830225f,
    0.00443184841f,
    0.0539909676f,
    0.241970733f,
    0.398942292f,
    0.241970733f,
    0.0539909676f,
    0.00443184841f,
    0.000133830225f,
    1.48671961e-06f
};

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);
    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    for (int i = 0, j = -5; i < 11; ++i, ++j)
    {
        result += weight[i] * imageLoad(inImage, t + ivec2(0, j)).rgb;
    }
    imageStore(outImage, t, vec4(result, 1.0f));
}
