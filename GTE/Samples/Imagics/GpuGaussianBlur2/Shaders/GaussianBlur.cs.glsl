// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(r32f) uniform readonly image2D inImage;
layout(r32f) uniform writeonly image2D outImage;

uniform Weight
{
    vec4 weight;  // (wx, wy, wz = 1 - 2*wx - 2*wy, unused)
};

layout(local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);

    // The plus-sign neighborhood is used for a finite difference
    // scheme that is based on solving the linear heat equation
    // with input equal to the image.
    float cZZ = imageLoad(inImage, t).r;
    float cPZ = imageLoad(inImage, t + ivec2(+1, 0)).r;
    float cMZ = imageLoad(inImage, t + ivec2(-1, 0)).r;
    float cZP = imageLoad(inImage, t + ivec2(0, +1)).r;
    float cZM = imageLoad(inImage, t + ivec2(0, -1)).r;
    float result =
        weight.x * (cPZ + cMZ) +
        weight.y * (cZP + cZM) +
        weight.z * cZZ;

    imageStore(outImage, t, vec4(result, 0, 0, 0));
}
