// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer ConstantColor
{
    float4 constantColor;
};

StructuredBuffer<float4> position;

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
    float4 modelPosition = position[input.id];
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, modelPosition);
#else
    output.clipPosition = mul(modelPosition, pvwMatrix);
#endif
    output.vertexColor = constantColor;
    return output;
}

