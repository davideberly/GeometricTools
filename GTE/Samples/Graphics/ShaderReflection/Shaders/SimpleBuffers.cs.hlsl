// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

Buffer<float> input;
RWBuffer<float4> output;

[numthreads(1, 1, 1)]
void CSMain(int3 t : SV_DispatchThreadID)
{
    output[0] = input[0] * float4(0.25f, 0.5f, 0.75f, 1.0f);
}


