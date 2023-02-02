// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Parameters
{
    vec4 spaceDelta;    // (dx, dy, dz, 0)
    vec4 halfDivDelta;  // (0.5/dx, 0.5/dy, 0.5/dz, 0)
    vec4 timeDelta;     // (dt/dx, dt/dy, dt/dz, dt)
    vec4 viscosityX;    // (velVX, velVX, velVX, denVX)
    vec4 viscosityY;    // (velVX, velVY, velVY, denVY)
    vec4 viscosityZ;    // (velVZ, velVZ, velVZ, denVZ)
    vec4 epsilon;       // (epsilonX, epsilonY, epsilonZ, epsilon0)
};

layout(rgba32f) uniform readonly image3D source;
layout(rgba32f) uniform readonly image3D stateT;
uniform sampler3D advectionSampler;
layout(rgba32f) uniform writeonly image3D updateState;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 dim = imageSize(stateT);

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
    vec4 stateZZZ = imageLoad(stateT, c);
    vec4 statePZZ = imageLoad(stateT, ivec3(xp, y, z));
    vec4 stateMZZ = imageLoad(stateT, ivec3(xm, y, z));
    vec4 stateZPZ = imageLoad(stateT, ivec3(x, yp, z));
    vec4 stateZMZ = imageLoad(stateT, ivec3(x, ym, z));
    vec4 stateZZP = imageLoad(stateT, ivec3(x, y, zp));
    vec4 stateZZM = imageLoad(stateT, ivec3(x, y, zm));

    // Sample the source state at (x,y,z).
    vec4 src = imageLoad(source, c);

    // Estimate second-order derivatives of state at (x,y,z).
    vec4 stateDXX = statePZZ - 2.0f * stateZZZ + stateMZZ;
    vec4 stateDYY = stateZPZ - 2.0f * stateZZZ + stateZMZ;
    vec4 stateDZZ = stateZZP - 2.0f * stateZZZ + stateZZM;

    // Compute advection.
    vec3 tcd = spaceDelta.xyz * (c - timeDelta.xyz * stateZZZ.xyz + 0.5f);
    vec4 advection = textureLod(advectionSampler, tcd, 0.0f);

    // Update the state.
    imageStore(updateState, c, advection +
        (viscosityX * stateDXX + viscosityY * stateDYY + viscosityZ * stateDZZ +
        timeDelta.w * src));
}
