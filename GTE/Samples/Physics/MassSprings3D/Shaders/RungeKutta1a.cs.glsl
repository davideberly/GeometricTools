// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform SimulationParameters
{
    ivec4 dimensions;    // (columns, rows, slices, columns*rows)
    float viscosity;
    float time;
    float delta;
    float halfDelta;    // delta/2
    float sixthDelta;   // delta/6
    float halfTime;     // time + halfDelta
    float fullTime;     // time + delta
};

buffer invMass      { float data[]; } invMassSB;
buffer constantC    { float data[]; } constantCSB;
buffer lengthC      { float data[]; } lengthCSB;
buffer constantR    { float data[]; } constantRSB;
buffer lengthR      { float data[]; } lengthRSB;
buffer constantS    { float data[]; } constantSSB;
buffer lengthS      { float data[]; } lengthSSB;

struct Temporary
{
    vec4 d1, d2, d3, d4;
};

buffer pAllTmp { Temporary data[]; } pAllTmpSB;  // packed dpTmp1, dpTmp2, dpTmp3, dpTmp4
buffer vAllTmp { Temporary data[]; } vAllTmpSB;  // packed dvTmp1, dvTmp2, dvTmp3, dvTmp4
buffer position { vec4 data[]; } positionSB;
buffer velocity { vec4 data[]; } velocitySB;

vec4 Acceleration(int i, ivec3 dt)
{
    vec4 diff, force;
    float ratio;
    int prev, next;

    // Initialize with the external acceleration.
    vec4 acc = -viscosity * velocitySB.data[i];

    if (dt.x > 0)
    {
        prev = i - 1;  // index to previous column
        diff = positionSB.data[prev] - positionSB.data[i];
        ratio = lengthCSB.data[prev] / length(diff);
        force = constantCSB.data[prev] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    if (dt.x < dimensions.x - 1)
    {
        next = i + 1;  // index to next column
        diff = positionSB.data[next] - positionSB.data[i];
        ratio = lengthCSB.data[i] / length(diff);
        force = constantCSB.data[i] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    if (dt.y > 0)
    {
        prev = i - dimensions.x;  // index to previous row
        diff = positionSB.data[prev] - positionSB.data[i];
        ratio = lengthRSB.data[prev] / length(diff);
        force = constantRSB.data[prev] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    if (dt.y < dimensions.y - 1)
    {
        next = i + dimensions.x;  // index to next row
        diff = positionSB.data[next] - positionSB.data[i];
        ratio = lengthRSB.data[i] / length(diff);
        force = constantRSB.data[i] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    if (dt.z > 0)
    {
        prev = i - dimensions.w;  // index to previous slice
        diff = positionSB.data[prev] - positionSB.data[i];
        ratio = lengthSSB.data[prev] / length(diff);
        force = constantSSB.data[prev] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    if (dt.z < dimensions.z - 1)
    {
        next = i + dimensions.w;  // index to next slice
        diff = positionSB.data[next] - positionSB.data[i];
        ratio = lengthSSB.data[i] / length(diff);
        force = constantSSB.data[i] * (1.0f - ratio) * diff;
        acc += invMassSB.data[i] * force;
    }

    return acc;
}

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 dt = ivec3(gl_GlobalInvocationID.xyz);
    int i = dt.x + dimensions.x * (dt.y + dimensions.y * dt.z);
    if (invMassSB.data[i] > 0.0f)
    {
        pAllTmpSB.data[i].d1 = velocitySB.data[i];
        vAllTmpSB.data[i].d1 = Acceleration(i, dt);
    }
}
