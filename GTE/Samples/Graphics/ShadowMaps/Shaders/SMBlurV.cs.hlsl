// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

Texture2D<float4> inImage;
RWTexture2D<float4> outImage;

static float weight[11] =
{
    1.48671961e-06f,
    0.000133830225f,
    0.00443184841f,
    0.0539909676f,
    0.241970733f,
    0.398942292f,
    0.241970733f,
    0.0539909676f,
    0.00443184841f,
    0.000133830225f,
    1.48671961e-06f
};

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 t : SV_DispatchThreadID)
{
    float3 result = 0.0f;
    [unroll]
    for (int i = 0, j = -5; i < 11; ++i, ++j)
    {
        result += weight[i] * inImage[t + int2(0, j)].rgb;
    }
    outImage[t] = float4(result, 1.0f);
}
