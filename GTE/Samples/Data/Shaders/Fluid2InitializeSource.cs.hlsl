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

cbuffer External
{
    float4 densityProducer;  // (x, y, variance, amplitude)
    float4 densityConsumer;  // (x, y, variance, amplitude)
    float4 gravity;          // (x, y, *, *)
    float4 wind;             // (x, y, variance, amplitude)
};

Texture2D<float2> vortexVelocity;
RWTexture2D<float4> source;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    // Compute the location of the pixel (x,y) in normalized [0,1]^2.
    float2 location = spaceDelta.xy * (c + 0.5f);

    // Compute an input to the fluid simulation consisting of a producer of
    // density and a consumer of density.
    float2 diff = location - densityProducer.xy;
    float arg = -dot(diff, diff) / densityProducer.z;
    float density = densityProducer.w * exp(arg);
    diff = location - densityConsumer.xy;
    arg = -dot(diff, diff) / densityConsumer.z;
    density -= densityConsumer.w * exp(arg);

    // Compute an input to the fluid simulation consisting of gravity,
    // a single wind source, and vortex impulses.
    float windDiff = location.y - wind.y;
    float windArg = -windDiff * windDiff / wind.z;
    float2 windVelocity = float2(wind.w * exp(windArg), 0.0f);
    float2 velocity = gravity.xy + windVelocity + vortexVelocity[c];

    source[c] = float4(velocity, 0.0f, density);
}
