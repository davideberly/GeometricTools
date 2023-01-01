// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

#include "RungeKutta.cs.hlsli"

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(int3 dt : SV_DispatchThreadID)
{
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMass[i] > 0.0f)
    {
        position[i] += sixthDelta * (pAllTmp[i].d1 +
            2.0f * (pAllTmp[i].d2 + pAllTmp[i].d3) + pAllTmp[i].d4);
        velocity[i] += sixthDelta * (vAllTmp[i].d1 +
            2.0f * (vAllTmp[i].d2 + vAllTmp[i].d3) + vAllTmp[i].d4);
    }
}
