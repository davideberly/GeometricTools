// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include "RungeKutta.cs.hlsli"

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        pAllTmp[i].d4 = velocity[i];
        vAllTmp[i].d4 = Acceleration(i, dt, dimensions, viscosity, invMass,
            pTmp, vTmp, constantC, lengthC, constantR, lengthR,
            constantS, lengthS);
    }
}

