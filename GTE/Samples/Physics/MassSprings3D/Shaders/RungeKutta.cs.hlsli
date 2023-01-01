// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer SimulationParameters
{
    int4 dimensions;    // (columns, rows, slices, columns*rows)
    float viscosity;
    float time;
    float delta;
    float halfDelta;    // delta/2
    float sixthDelta;   // delta/6
    float halfTime;     // time + halfDelta
    float fullTime;     // time + delta
};

StructuredBuffer<float> invMass;
StructuredBuffer<float> constantC;
StructuredBuffer<float> lengthC;
StructuredBuffer<float> constantR;
StructuredBuffer<float> lengthR;
StructuredBuffer<float> constantS;
StructuredBuffer<float> lengthS;

struct Temporary
{
    float4 d1, d2, d3, d4;
};

RWStructuredBuffer<float4> pTmp;
RWStructuredBuffer<Temporary> pAllTmp;  // packed dpTmp1, dpTmp2, dpTmp3, dpTmp4
RWStructuredBuffer<float4> vTmp;
RWStructuredBuffer<Temporary> vAllTmp;  // packed dvTmp1, dvTmp2, dvTmp3, dvTmp4
RWStructuredBuffer<float4> position;
RWStructuredBuffer<float4> velocity;

float4 Acceleration(int i, int3 dt, int4 dimensions,
    float viscosity, StructuredBuffer<float> invMass,
    RWStructuredBuffer<float4> position, RWStructuredBuffer<float4> velocity,
    StructuredBuffer<float> constantC, StructuredBuffer<float> lengthC,
    StructuredBuffer<float> constantR, StructuredBuffer<float> lengthR,
    StructuredBuffer<float> constantS, StructuredBuffer<float> lengthS)
{
    float4 diff, force;
    float ratio;
    int prev, next;

    // Initialize with the external acceleration.
    float4 acc = -viscosity * velocity[i];

    if (dt.x > 0)
    {
        prev = i - 1;  // index to previous column
        diff = position[prev] - position[i];
        ratio = lengthC[prev] / length(diff);
        force = constantC[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.x < dimensions.x - 1)
    {
        next = i + 1;  // index to next column
        diff = position[next] - position[i];
        ratio = lengthC[i] / length(diff);
        force = constantC[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.y > 0)
    {
        prev = i - dimensions.x;  // index to previous row
        diff = position[prev] - position[i];
        ratio = lengthR[prev] / length(diff);
        force = constantR[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.y < dimensions.y - 1)
    {
        next = i + dimensions.x;  // index to next row
        diff = position[next] - position[i];
        ratio = lengthR[i] / length(diff);
        force = constantR[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.z > 0)
    {
        prev = i - dimensions.w;  // index to previous slice
        diff = position[prev] - position[i];
        ratio = lengthS[prev] / length(diff);
        force = constantS[prev] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    if (dt.z < dimensions.z - 1)
    {
        next = i + dimensions.w;  // index to next slice
        diff = position[next] - position[i];
        ratio = lengthS[i] / length(diff);
        force = constantS[i] * (1.0f - ratio) * diff;
        acc += invMass[i] * force;
    }

    return acc;
}
