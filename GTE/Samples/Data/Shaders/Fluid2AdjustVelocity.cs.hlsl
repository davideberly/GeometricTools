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

Texture2D<float4> inState;
Texture2D<float> poisson;
RWTexture2D<float4> outState;

[numthreads(NUM_X_THREADS, NUM_Y_THREADS, 1)]
void CSMain(uint2 c : SV_DispatchThreadID)
{
    uint2 dim;
    inState.GetDimensions(dim.x, dim.y);

    int x = int(c.x);
    int y = int(c.y);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);

    // Sample the state at (x,y).
    float4 state = inState[c];

    // Sample Poisson values at immediate neighbors of (x,y).
    float poisPZ = poisson[int2(xp, y)];
    float poisMZ = poisson[int2(xm, y)];
    float poisZP = poisson[int2(x, yp)];
    float poisZM = poisson[int2(x, ym)];

    float4 diff = float4(poisPZ - poisMZ, poisZP - poisZM, 0.0f, 0.0f);
    outState[c] = state + halfDivDelta * diff;
}
