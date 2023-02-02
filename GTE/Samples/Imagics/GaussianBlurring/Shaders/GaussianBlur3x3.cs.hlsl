// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float4> inImage;
RWTexture2D<float4> outImage;

static float weight[3][3] =
{
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
    { 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f },
    { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f }
};

static int2 offset[3][3] =
{
    { int2(-1, -1), int2(0, -1), int2(+1, -1) },
    { int2(-1,  0), int2(0,  0), int2(+1,  0) },
    { int2(-1, +1), int2(0, +1), int2(+1, +1) }
};

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    float4 result = 0.0f;
    [unroll]
    for (int r = 0; r < 3; ++r)
    {
        [unroll]
        for (int c = 0; c < 3; ++c)
        {
            result += weight[r][c] * inImage[t + offset[r][c]];
        }
    }
    outImage[t] = float4(result.rgb, 1.0f);
}
