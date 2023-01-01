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

Texture2D<float> random;
RWTexture2D<float4> weights;

// Compute the height using a bicubic polynomial that does not have the convex
// hull property.  Clamp below to zero but scale down rather than saturate to
// avoid flat spots in the graph.
float GetHeight(float2 uv)
{
    float2 omuv = 1.0f - uv;
    float2 uv2 = uv * uv;
    float2 omuv2 = omuv * omuv;
    float2 uvpoly[4] = { omuv2*omuv, 3.0f*omuv2*uv, 3.0f*omuv*uv2, uv*uv2 };

    float product[4];
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        product[i] = 0.0f;
        [unroll]
        for (int j = 0; j < 4; ++j)
        {
            product[i] += uvpoly[j].y * control[i][j];
        }
    }

    float height = 0.0f;
    [unroll]
    for (int k = 0; k < 4; ++k)
    {
        height += uvpoly[k].x * product[k];
    }
    height = max(0.5f*height, 0.0f);
    return height;
}

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(int2 dt : SV_DispatchThreadID)
{
    uint2 dims;
    weights.GetDimensions(dims.x, dims.y);
    float2 fdims = float2(dims);

    float height00 = GetHeight(float2(dt) / fdims) + random[dt];
    int2 dt10 = dt + int2(1, 0);
    float height10 = GetHeight(float2(dt10) / fdims) + random[dt10];
    int2 dt01 = dt + int2(0, 1);
    float height01 = GetHeight(float2(dt01) / fdims) + random[dt01];
    int2 dt11 = dt + int2(1, 1);
    float height11 = GetHeight(float2(dt11) / fdims) + random[dt11];
    float weight1 = 0.5f*(height00 + height10);
    float weight2 = 0.5f*(height00 + height01);
    float weight3 = 0.70710678f*(height00 + height11);
    weights[dt] = float4(height00, weight1, weight2, weight3);
}
