// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct A
{
    float fvalue[4];
    int2 i2value;
};

struct B
{
    int ivalue;
    A avalue;
};

cbuffer MyCBuffer
{
    B input;
};

StructuredBuffer<B> sbuffer[2];
Texture2D<float4> mytexture;

RWTexture1D<float> output;

[numthreads(1, 1, 1)]
void CSMain(int t : SV_DispatchThreadID)
{
    float result = (float)input.ivalue;
    for (int i = 0; i < 4; ++i)
    {
        result += input.avalue.fvalue[i];
    }
    result += (float)input.avalue.i2value.x;
    result += (float)input.avalue.i2value.y;

    [unroll]
    for (int j = 0; j < 2; ++j)
    {
        B mybvalue = sbuffer[j][0];
        result += (float)mybvalue.ivalue;
        for (int k = 0; k < 4; ++k)
        {
            result += mybvalue.avalue.fvalue[k];
        }
        result += (float)mybvalue.avalue.i2value.x;
        result += (float)mybvalue.avalue.i2value.y;
    }

    result += mytexture[int2(0,0)].x;

    output[0] = result;
}
