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
    float4 timeDelta;     // (dt, dt/dx, dt/dy, 0)
    float4 viscosityX;    // (velVX, velVX, 0, denVX)
    float4 viscosityY;    // (velVX, velVY, 0, denVY)
    float4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
};

Texture2D<float4> state;
RWTexture2D<float> divergence;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    state.GetDimensions(dim.x, dim.y);

    int x = int(c.x);
    int y = int(c.y);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);

    float2 velocityGradient = float2(
        state[int2(xp, y)].x - state[int2(xm, y)].x,
        state[int2(x, yp)].y - state[int2(x, ym)].y
    );

    divergence[c] = dot(halfDivDelta.xy, velocityGradient);
}
