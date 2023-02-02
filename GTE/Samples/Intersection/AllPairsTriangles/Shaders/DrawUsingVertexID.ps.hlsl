// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

static float4 color[4] =
{
    float4(0.0f, 0.0f, 0.5f, 1.0f),
    float4(0.5f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 0.5f, 0.5f, 1.0f),
    float4(0.5f, 0.5f, 0.0f, 1.0f)
};

StructuredBuffer<float4> positions;
StructuredBuffer<uint> colorIndices;

struct VS_INPUT
{
    uint id : SV_VertexID;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, positions[input.id]);
#else
    output.clipPosition = mul(positions[input.id], pvwMatrix);
#endif
    output.vertexColor = color[colorIndices[input.id]];
    return output;
}

struct PS_INPUT
{
    float4 vertexColor : COLOR0;
};

struct PS_OUTPUT
{
    float4 pixelColor0 : SV_TARGET0;
};

PS_OUTPUT PSMain(PS_INPUT input)
{
    PS_OUTPUT output;
    output.pixelColor0 = input.vertexColor;
    return output;
}
