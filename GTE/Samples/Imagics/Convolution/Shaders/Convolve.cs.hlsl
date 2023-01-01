// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Weights
{
    float weight[2 * RADIUS + 1][2 * RADIUS + 1];
};

Texture2D<float4> inImage;
RWTexture2D<float4> outImage;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    float4 result = 0.0f;
    for (int y = -RADIUS; y <= RADIUS; ++y)
    {
        for (int x = -RADIUS; x <= RADIUS; ++x)
        {
            result += weight[y + RADIUS][x + RADIUS] * inImage[dt + int2(x, y)];
        }
    }
    outImage[dt] = result;
}
