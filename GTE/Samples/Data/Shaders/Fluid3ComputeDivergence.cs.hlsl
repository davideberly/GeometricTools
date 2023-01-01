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

Texture3D<float4> state;
RWTexture3D<float> divergence;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    uint3 dim;
    state.GetDimensions(dim.x, dim.y, dim.z);

    int x = int(c.x);
    int y = int(c.y);
    int z = int(c.z);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);
    int zm = max(z - 1, 0);
    int zp = min(z + 1, dim.z - 1);

    float3 velocityGradient = float3
    (
        state[int3(xp, y, z)].x - state[int3(xm, y, z)].x,
        state[int3(x, yp, z)].y - state[int3(x, ym, z)].y,
        state[int3(x, y, zp)].z - state[int3(x, y, zm)].z
    );

    divergence[c] = dot(halfDivDelta.xyz, velocityGradient);
}
