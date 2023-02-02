// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Parameters
{
    vec4 spaceDelta;    // (dx, dy, 0, 0)
    vec4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0, 0)
    vec4 timeDelta;     // (dt/dx, dt/dy, 0, dt)
    vec4 viscosityX;    // (velVX, velVX, 0, denVX)
    vec4 viscosityY;    // (velVX, velVY, 0, denVY)
    vec4 epsilon;       // (epsilonX, epsilonY, 0, epsilon0)
};

layout(rgba32f) uniform readonly image2D source;
layout(rgba32f) uniform readonly image2D stateT;
uniform sampler2D stateTm1;
layout(rgba32f) uniform writeonly image2D updateState;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(stateT);

    int x = int(c.x);
    int y = int(c.y);
    int xm = max(x - 1, 0);
    int xp = min(x + 1, dim.x - 1);
    int ym = max(y - 1, 0);
    int yp = min(y + 1, dim.y - 1);

    // Sample states at (x,y), (x+dx,y), (x-dx,y), (x,y+dy), (x,y-dy).
    vec4 stateZZ = imageLoad(stateT, c);
    vec4 statePZ = imageLoad(stateT, ivec2(xp, y));
    vec4 stateMZ = imageLoad(stateT, ivec2(xm, y));
    vec4 stateZP = imageLoad(stateT, ivec2(x, yp));
    vec4 stateZM = imageLoad(stateT, ivec2(x, ym));

    // Sample the source state at (x,y).
    vec4 src = imageLoad(source, c);

    // Estimate second-order derivatives of state at (x,y).
    vec4 stateDXX = statePZ - 2.0f * stateZZ + stateMZ;
    vec4 stateDYY = stateZP - 2.0f * stateZZ + stateZM;

    // Compute advection.
    vec2 tcd = spaceDelta.xy * (c - timeDelta.xy * stateZZ.xy + 0.5f);
    vec4 advection = textureLod(stateTm1, tcd, 0.0f);

    // Update the state.
    imageStore(updateState, c, advection +
        (viscosityX * stateDXX + viscosityY * stateDYY + timeDelta.w * src));
};
