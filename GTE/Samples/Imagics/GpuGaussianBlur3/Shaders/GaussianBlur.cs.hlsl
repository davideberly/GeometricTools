// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

// The 3D image is stored as a tiled 2D texture, in which case the selection
// of z-neighboring pixels is handled by using a precomputed offset texture.

Texture2D<float> inImage;
Texture2D<int4> inZNeighbor;
RWTexture2D<float> outImage;

cbuffer Weight
{
    float4 weight;  // (wx, wy, wz, ww = 1 - 2*wx - 2*wy - 2*wz)
};

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    // The plus-sign neighborhood is used for a finite difference
    // scheme that is based on solving the linear heat equation
    // with input equal to the image.
    int4 neighbor = inZNeighbor[t];
    float cZZZ = inImage[t];                // image(x,y,z)
    float cPZZ = inImage[t + int2(+1, 0)];  // image(x+1,y,z)
    float cMZZ = inImage[t + int2(-1, 0)];  // image(x-1,y,z)
    float cZPZ = inImage[t + int2(0, +1)];  // image(x,y+1,z)
    float cZMZ = inImage[t + int2(0, -1)];  // image(x,y-1,z)
    float cZZP = inImage[t + neighbor.xy];  // image(x,y,z+1)
    float cZZM = inImage[t + neighbor.zw];  // image(x,y,z-1)

    outImage[t] =
        weight.x * (cPZZ + cMZZ) +
        weight.y * (cZPZ + cZMZ) +
        weight.z * (cZZP + cZZM) +
        weight.w * cZZZ;
}
