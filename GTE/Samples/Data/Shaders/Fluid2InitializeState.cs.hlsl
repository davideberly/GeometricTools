// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float> density;
Texture2D<float2> velocity;
RWTexture2D<float4> stateTm1;
RWTexture2D<float4> stateT;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    float4 initial = float4(velocity[c], 0.0f, density[c]);
    stateTm1[c] = initial;
    stateT[c] = initial;
}
