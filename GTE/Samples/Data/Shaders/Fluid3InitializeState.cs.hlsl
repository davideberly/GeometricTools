// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture3D<float> density;
Texture3D<float4> velocity;
RWTexture3D<float4> stateTm1;
RWTexture3D<float4> stateT;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    float4 initial = float4(velocity[c.xyz].xyz, density[c.xyz]);
    stateTm1[c.xyz] = initial;
    stateT[c.xyz] = initial;
}
