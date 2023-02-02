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

cbuffer Vortex
{
    float4 position;  // (px, py, pz, *)
    float4 normal;    // (nx, ny, nz, *)
    float4 data;      // (variance, amplitude, *, *)
};

Texture3D<float4> inVelocity;
RWTexture3D<float4> outVelocity;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    float3 location = spaceDelta.xyz * (c.xyz + 0.5f);
    float3 diff = location - position.xyz;
    float arg = -dot(diff, diff) / data.x;
    float magnitude = data.y * exp(arg);
    float4 vortexVelocity = float4(magnitude * cross(normal.xyz, diff), 0.0f);
    outVelocity[c.xyz] = inVelocity[c.xyz] + vortexVelocity;
}
