// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct Particle
{
    float4 position;   // (x,y,z,1)
    float4 colorSize;  // (r,g,b;size)
};

StructuredBuffer<Particle> particles;

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

struct GS_INPUT
{
    uint id : TEXCOORD0;
};

struct GS_OUTPUT
{
    float3 color : COLOR0;
    float4 clipPosition : SV_POSITION;
};

[maxvertexcount(6)]
void GSMain (point GS_INPUT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
    Particle particle = particles[input[0].id];

    GS_OUTPUT output[4];
#if GTE_USE_MAT_VEC
    float4 viewPosition = mul(vwMatrix, particle.position);
#else
    float4 viewPosition = mul(particle.position, vwMatrix);
#endif
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float4 corner = viewPosition + particle.colorSize.a * offset[i];
#if GTE_USE_MAT_VEC
        output[i].clipPosition = mul(pMatrix, corner);
#else
        output[i].clipPosition = mul(corner, pMatrix);
#endif
        output[i].color = particle.colorSize.rgb;
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
