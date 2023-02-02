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

uniform Vortex
{
    vec4 position;  // (px, py, pz, *)
    vec4 normal;    // (nx, ny, nz, *)
    vec4 data;      // (variance, amplitude, *, *)
};

layout(rgba32f) uniform readonly image3D inVelocity;
layout(rgba32f) uniform writeonly image3D outVelocity;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);
    vec3 location = spaceDelta.xyz * (c + 0.5f);
    vec3 diff = location - position.xyz;
    float arg = -dot(diff, diff) / data.x;
    float magnitude = data.y * exp(arg);
    vec4 vortexVelocity = vec4(magnitude * cross(normal.xyz, diff), 0.0f);
    imageStore(outVelocity, c, imageLoad(inVelocity, c) + vortexVelocity);
}
