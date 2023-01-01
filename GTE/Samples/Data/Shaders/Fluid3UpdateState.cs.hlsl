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

Texture3D<float4> source;
Texture3D<float4> stateTm1;
Texture3D<float4> stateT;
SamplerState advectionSampler;  // trilinear, clamp
RWTexture3D<float4> updateState;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, NUM_Z_THREADS)]
void CSMain(uint3 c : SV_DispatchThreadID)
{
    uint3 dim;
    stateT.GetDimensions(dim.x, dim.y, dim.z);

    int x = int(c.x);
    int y = int(c.y);
    int z = int(c.z);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);
    int zm = max(z - 1, 0);
    int zp = min(z + 1, dim.z - 1);

    // Sample states at (x,y,z) and immediate neighbors.
    float4 stateZZZ = stateT[int3(x, y, z)];
    float4 statePZZ = stateT[int3(xp, y, z)];
    float4 stateMZZ = stateT[int3(xm, y, z)];
    float4 stateZPZ = stateT[int3(x, yp, z)];
    float4 stateZMZ = stateT[int3(x, ym, z)];
    float4 stateZZP = stateT[int3(x, y, zp)];
    float4 stateZZM = stateT[int3(x, y, zm)];

    // Sample the source state at (x,y,z).
    float4 src = source[int3(x, y, z)];

    // Estimate second-order derivatives of state at (x,y,z).
    float4 stateDXX = statePZZ - 2.0f * stateZZZ + stateMZZ;
    float4 stateDYY = stateZPZ - 2.0f * stateZZZ + stateZMZ;
    float4 stateDZZ = stateZZP - 2.0f * stateZZZ + stateZZM;

    // Compute advection.
    float3 tcd = spaceDelta.xyz*(c.xyz - timeDelta.xyz * stateZZZ.xyz + 0.5f);
    float4 advection = stateTm1.SampleLevel(advectionSampler, tcd, 0.0f);

    // Update the state.
    updateState[c.xyz] = advection +
        (viscosityX * stateDXX + viscosityY * stateDYY + +viscosityZ * stateDZZ +
        timeDelta.w * src);
}
