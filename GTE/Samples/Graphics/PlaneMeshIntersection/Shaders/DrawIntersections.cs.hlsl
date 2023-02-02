// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

Texture2D<float4> colorImage;
Texture2D<float2> planeConstantImage;
RWTexture2D<float4> outputImage;

[numthreads(8,8,1)]
void CSMain(int2 t : SV_DISPATCHTHREADID)
{
    uint value = asuint(colorImage[t].w);
    if (value == 0x7F7FFFFF)
    {
        outputImage[t] = float4(colorImage[t].rgb, 1.0f);
        return;
    }

    float signChange = 0.0f;
    float2 intValue = floor(planeConstantImage[t]);
    float2 diff0 = planeConstantImage[t] - intValue;
    float2 diff1 = 0.0f;
    if (diff0.x > 0.0f || diff0.y > 0.0f)
    {
        int2 nbr;
        for (int dy = -1; dy <= 1; ++dy)
        {
            nbr.y = t.y + dy;
            for (int dx = -1; dx <= 1; ++dx)
            {
                nbr.x = t.x + dx;
                diff1 = intValue - planeConstantImage[nbr];
                if (diff1.x >= 0.0f || diff1.y >= 0.0f)
                {
                    signChange = 1.0f;
                }
            }
        }
    }
    else
    {
        signChange = 1.0f;
    }

    outputImage[t] = signChange * float4(0.0f, 0.0f, 0.0f, 1.0f) +
        (1.0f - signChange) * float4(colorImage[t].rgb, 1.0f);
}
