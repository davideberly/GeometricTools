// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct GS_INPUT
{
    float3 position : POSITION;
    float3 color : COLOR0;
    float size : TEXCOORD0;
};

struct GS_OUTPUT
{
    float3 color : COLOR0;
    float4 clipPosition : SV_POSITION;
};

cbuffer Matrices
{
    float4x4 vwMatrix;
    float4x4 pMatrix;
};

static float4 offset[4] =
{
    float4(-1.0f, -1.0f, 0.0f, 0.0f),
    float4(+1.0f, -1.0f, 0.0f, 0.0f),
    float4(-1.0f, +1.0f, 0.0f, 0.0f),
    float4(+1.0f, +1.0f, 0.0f, 0.0f)
};

[maxvertexcount(6)]
void GSMain (point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[4];
    float4 viewPosition = mul(vwMatrix, float4(input[0].position, 1.0f));
    int i;
    for (i = 0; i < 4; ++i)
    {
        float4 corner = viewPosition + input[0].size*offset[i];
        output[i].clipPosition = mul(pMatrix, corner);
        output[i].color = input[0].color;
    }

    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[3]);
    stream.RestartStrip();

    stream.Append(output[0]);
    stream.Append(output[3]);
    stream.Append(output[2]);
    stream.RestartStrip();
}
