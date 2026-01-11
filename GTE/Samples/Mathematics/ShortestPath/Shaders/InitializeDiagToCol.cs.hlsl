// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

Texture2D<float4> weights;
RWTexture2D<int2> previous;
RWTexture2D<float> sum;

[numthreads(ISIZE, 1, 1)]
void CSMain(int diagonal : SV_GroupThreadID)
{
    previous[int2(0, diagonal)] = int2(0, diagonal - 1);
    sum[int2(diagonal, diagonal)] = weights[int2(0, diagonal)].z;
}


