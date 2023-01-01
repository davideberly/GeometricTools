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
        pTmp[i] = position[i] + halfDelta * pAllTmp[i].d2;
        vTmp[i] = velocity[i] + halfDelta * vAllTmp[i].d2;
    }
    else
    {
        pTmp[i] = position[i];
        vTmp[i] = 0.0f;
    }
}
