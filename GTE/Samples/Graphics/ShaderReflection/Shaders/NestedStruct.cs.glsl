// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct A
{
    float fvalue[4];
    ivec2 i2value;
};

struct B
{
    int ivalue;
    A avalue;
};

uniform MyCBuffer
{
    B inBuf;
};

buffer StructuredBuffer
{
    B data[];
} sbuffer[2];

layout(rgba32f) uniform readonly image2D mytexture;
layout(r32f) uniform writeonly image1D myoutput;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    ivec3 t = ivec3(gl_GlobalInvocationID.xyz);

    float result = float(inBuf.ivalue);
    for (int i = 0; i < 4; ++i)
    {
        result += inBuf.avalue.fvalue[i];
    }
    result += float(inBuf.avalue.i2value.x);
    result += float(inBuf.avalue.i2value.y);

    for (int j = 0; j < 2; ++j)
    {
        B mybvalue = sbuffer[j].data[0];
        result += float(mybvalue.ivalue);
        for (int k = 0; k < 4; ++k)
        {
            result += mybvalue.avalue.fvalue[k];
        }
        result += float(mybvalue.avalue.i2value.x);
        result += float(mybvalue.avalue.i2value.y);
    }

    result += imageLoad(mytexture, ivec2(0,0)).x;

    imageStore(myoutput, 0, vec4(result, 0, 0, 0));
}
