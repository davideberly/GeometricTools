// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer WireParameters
{
    float4 meshColor;
    float4 edgeColor;
    float2 windowSize;
};

struct GS_INPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

struct GS_OUTPUT
{
    float4 vertexColor : COLOR0;
    noperspective float3 edgeDistance : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

[maxvertexcount(3)]
void GSMain(triangle GS_INPUT input[3], inout TriangleStream<GS_OUTPUT> stream)
{
    GS_OUTPUT output[3];

    float2 pixel[3];
    int i;
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        float2 ndc = input[i].clipPosition.xy / input[i].clipPosition.w;
        pixel[i] = 0.5f * windowSize * (ndc + 1.0f);
    }

    int j0[3] = { 2, 0, 1 }, j1[3] = { 1, 2, 0 };
    float edgeDistance[3];
    [unroll]
    for (i = 0; i < 3; ++i)
    {
        float2 diff = pixel[i] - pixel[j1[i]];
        float2 edge = pixel[j0[i]] - pixel[j1[i]];
        float edgeLength = length(edge);
        if (edgeLength > 0.0f)
        {
            edgeDistance[i] =
                abs(dot(diff, float2(edge.y, -edge.x)) / edgeLength);
        }
        else
        {
            edgeDistance[i] = 0.0f;
        }

        output[i].vertexColor = input[i].vertexColor;
        output[i].clipPosition = input[i].clipPosition;
    }

    output[0].edgeDistance = float3(edgeDistance[0], 0.0f, 0.0f);
    output[1].edgeDistance = float3(0.0f, edgeDistance[1], 0.0f);
    output[2].edgeDistance = float3(0.0f, 0.0f, edgeDistance[2]);
    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[2]);
    stream.RestartStrip();
}
