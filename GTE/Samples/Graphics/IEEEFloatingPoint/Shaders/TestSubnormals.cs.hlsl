// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

StructuredBuffer<REAL> inBuffer;  // Two subnormal numbers.
RWStructuredBuffer<REAL> outBuffer;  // The sum of inputs, supposed to be subnormal.

[numthreads(1, 1, 1)]
void CSMain(int3 t : SV_DispatchThreadID)
{
    outBuffer[0] = inBuffer[0] + inBuffer[1];
}

