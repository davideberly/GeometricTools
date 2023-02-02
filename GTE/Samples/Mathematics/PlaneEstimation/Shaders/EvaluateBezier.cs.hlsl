// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer ControlPoints
{
    float4 control[4];
};

RWTexture2D<float4> positions;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 t : SV_DispatchThreadID)
{
    uint2 dims;
    positions.GetDimensions(dims.x, dims.y);

    float2 uv = float2(t) / float2(dims);
    float2 omuv = 1.0f - uv;
    float2 uv2 = uv * uv;
    float2 omuv2 = omuv * omuv;
    float2 uvpoly[4] = { omuv2 * omuv, 3.0f * omuv2 * uv, 3.0f * omuv * uv2, uv * uv2 };

    float product[4];
    int i, j;

    [unroll]
    for (i = 0; i < 4; ++i)
    {
        product[i] = 0.0f;
        [unroll]
        for (j = 0; j < 4; ++j)
        {
            product[i] += uvpoly[j].y * control[i][j];
        }
    }

    float height = 0.0f;
    [unroll]
    for (i = 0; i < 4; ++i)
    {
        height += uvpoly[i].x * product[i];
    }

    if (height > 0.0f)
    {
        positions[t] = float4(float2(t), height, 1.0f);
    }
    else
    {
        positions[t] = float4(float2(t), 0.0f, 0.0f);
    }
}
