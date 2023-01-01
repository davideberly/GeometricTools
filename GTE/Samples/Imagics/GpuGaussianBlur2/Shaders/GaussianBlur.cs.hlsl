// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float> inImage;
RWTexture2D<float> outImage;

cbuffer Weight
{
    float4 weight;  // (wx, wy, wz = 1 - 2*wx - 2*wy, unused)
};

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    // The plus-sign neighborhood is used for a finite difference scheme that
    // is based on solving the linear heat equation with input equal to the
    // image.
    float cZZ = inImage[t];
    float cPZ = inImage[t + int2(+1, 0)];
    float cMZ = inImage[t + int2(-1, 0)];
    float cZP = inImage[t + int2(0, +1)];
    float cZM = inImage[t + int2(0, -1)];

    outImage[t] =
        weight.x * (cPZ + cMZ) +
        weight.y * (cZP + cZM) +
        weight.z * cZZ;
}
