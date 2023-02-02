// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

RWStructuredBuffer<uint> color0;
RWStructuredBuffer<uint> color1;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    [unroll]
    for (int j = 0; j < 3; ++j)
    {
        color0[3*dt.x + j] = 0;
        color1[3*dt.y + j] = 1;
    }
}
