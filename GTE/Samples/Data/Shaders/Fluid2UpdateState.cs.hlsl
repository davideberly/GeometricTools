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

Texture2D<float4> source;
Texture2D<float4> stateTm1;
Texture2D<float4> stateT;
SamplerState advectionSampler;  // bilinear, clamp
RWTexture2D<float4> updateState;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    stateT.GetDimensions(dim.x, dim.y);

    int x = int(c.x);
    int y = int(c.y);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);

    // Sample states at (x,y), (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
    float4 stateZZ = stateT[int2(x, y)];
    float4 statePZ = stateT[int2(xp, y)];
    float4 stateMZ = stateT[int2(xm, y)];
    float4 stateZP = stateT[int2(x, yp)];
    float4 stateZM = stateT[int2(x, ym)];

    // Sample the source state at (x,y).
    float4 src = source[int2(x, y)];

    // Estimate second-order derivatives of state at (x,y).
    float4 stateDXX = statePZ - 2.0f * stateZZ + stateMZ;
    float4 stateDYY = stateZP - 2.0f * stateZZ + stateZM;

    // Compute advection.
    float2 tcd = spaceDelta.xy * (c - timeDelta.xy * stateZZ.xy + 0.5f);
    float4 advection = stateTm1.SampleLevel(advectionSampler, tcd, 0.0f);

    // Update the state.
    updateState[c] = advection +
        (viscosityX * stateDXX + viscosityY * stateDYY + timeDelta.w * src);
}
