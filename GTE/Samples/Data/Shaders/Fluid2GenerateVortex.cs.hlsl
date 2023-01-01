// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Parameters
{
    float4 spaceDelta;    // (dx, dy, 0, 0)
    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)
    float4 timeDelta;     // (dt/dx, dt/dy, 0, dt)
    float4 viscosityX;    // (velVX, velVX, 0, denVX)
    float4 viscosityY;    // (velVX, velVY, 0, denVY)
    float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
};

cbuffer Vortex
{
    float4 data;  // (x, y, variance, amplitude)
};

Texture2D<float2> inVelocity;
RWTexture2D<float2> outVelocity;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    float2 location = spaceDelta.xy * (c.xy + 0.5f);
    float2 diff = location - data.xy;
    float arg = -dot(diff, diff) / data.z;
    float magnitude = data.w * exp(arg);
    float2 vortexVelocity = magnitude * float2(diff.y, -diff.x);
    outVelocity[c.xy] = inVelocity[c.xy] + vortexVelocity;
}
