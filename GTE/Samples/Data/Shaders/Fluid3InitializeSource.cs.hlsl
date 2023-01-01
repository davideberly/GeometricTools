// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer Parameters
{
    float4 spaceDelta;    // (dx, dy, dz, 0)
    float4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0.5/dz, 0)
    float4 timeDelta;     // (dt/dx, dt/dy, dt/dz, dt)
    float4 viscosityX;    // (velVX, velVX, velVX, denVX)
    float4 viscosityY;    // (velVX, velVY, velVY, denVY)
    float4 viscosityZ;    // (velVZ, velVZ, velVZ, denVZ)
    float4 epsilon;       // (epsilonX, epsilonY, epsilonZ, epsilon0)
};

cbuffer External
{
    float4 densityProducer;  // (x, y, z, *)
    float4 densityPData;     // (variance, amplitude, *, *)
    float4 densityConsumer;  // (x, y, z, *)
    float4 densityCData;     // (variance, amplitude, *, *)
    float4 gravity;          // (x, y, z, *)
    float4 windData;         // (variance, amplitude, *, *)
};

Texture3D<float4> vortexVelocity;
RWTexture3D<float4> source;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    // Compute the location of the voxel (x,y,z) in normalized [0,1]^3.
    float3 location = spaceDelta.xyz * (c.xyz + 0.5f);

    // Compute an input to the fluid simulation consisting of a producer of
    // density and a consumer of density.
    float3 diff = location - densityProducer.xyz;
    float arg = -dot(diff, diff) / densityPData.x;
    float density = densityPData.y * exp(arg);
    diff = location - densityConsumer.xyz;
    arg = -dot(diff, diff) / densityCData.x;
    density -= densityCData.y * exp(arg);

    // Compute an input to the fluid simulation consisting of gravity,
    // a single wind source, and vortex impulses.
    float windArg = -dot(location.xz, location.xz) / windData.x;
    float3 windVelocity = float3(0.0f, windData.y * exp(windArg), 0.0f);
    float3 velocity = gravity.xyz + windVelocity + vortexVelocity[c.xyz].xyz;

    source[c.xyz] = float4(velocity.xyz, density);
}
