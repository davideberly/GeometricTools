// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

RWTexture2D<float> poisson;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    poisson[c] = 0.0f;
}
