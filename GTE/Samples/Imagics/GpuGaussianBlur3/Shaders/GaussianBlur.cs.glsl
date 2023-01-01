// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(r32f) uniform readonly image2D inImage;
layout(rgba32i) uniform readonly iimage2D inZNeighbor;
layout(r32f) uniform writeonly image2D outImage;

uniform Weight
{
    vec4 weight;  // (wx, wy, wz, ww = 1 - 2*wx - 2*wy - 2*wz)
};

layout(local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 t = ivec2(gl_GlobalInvocationID.xy);

    // The plus-sign neighborhood is used for a finite difference
    // scheme that is based on solving the linear heat equation
    // with input equal to the image.
    ivec4 neighbor = imageLoad(inZNeighbor, t);
    float cZZZ = imageLoad(inImage, t).r;                   // image(x,y,z)
    float cPZZ = imageLoad(inImage, t + ivec2(+1, 0)).r;    // image(x+1,y,z)
    float cMZZ = imageLoad(inImage, t + ivec2(-1, 0)).r;    // image(x-1,y,z)
    float cZPZ = imageLoad(inImage, t + ivec2(0, +1)).r;    // image(x,y+1,z)
    float cZMZ = imageLoad(inImage, t + ivec2(0, -1)).r;    // image(x,y-1,z)
    float cZZP = imageLoad(inImage, t + neighbor.xy).r;     // image(x,y,z+1)
    float cZZM = imageLoad(inImage, t + neighbor.zw).r;     // image(x,y,z-1)
    float result =
        weight.x * (cPZZ + cMZZ) +
        weight.y * (cZPZ + cZMZ) +
        weight.z * (cZZP + cZZM) +
        weight.w * cZZZ;

    imageStore(outImage, t, vec4(result, 0, 0, 0));
}
