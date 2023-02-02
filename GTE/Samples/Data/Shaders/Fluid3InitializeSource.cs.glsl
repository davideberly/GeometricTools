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

uniform External
{
    vec4 densityProducer;  // (x, y, z, *)
    vec4 densityPData;     // (variance, amplitude, *, *)
    vec4 densityConsumer;  // (x, y, z, *)
    vec4 densityCData;     // (variance, amplitude, *, *)
    vec4 gravity;          // (x, y, z, *)
    vec4 windData;         // (variance, amplitude, *, *)
};

layout(rgba32f) uniform readonly image3D vortexVelocity;
layout(rgba32f) uniform writeonly image3D source;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = NUM_Z_THREADS) in;
void main()
{
    ivec3 c = ivec3(gl_GlobalInvocationID.xyz);

    // Compute the location of the voxel (x,y,z) in normalized [0,1]^3.
    vec3 location = spaceDelta.xyz * (c + 0.5f);

    // Compute an input to the fluid simulation consisting of a producer of
    // density and a consumer of density.
    vec3 diff = location - densityProducer.xyz;
    float arg = -dot(diff, diff) / densityPData.x;
    float density = densityPData.y * exp(arg);
    diff = location - densityConsumer.xyz;
    arg = -dot(diff, diff) / densityCData.x;
    density -= densityCData.y * exp(arg);

    // Compute an input to the fluid simulation consisting of gravity,
    // a single wind source, and vortex impulses.
    float windArg = -dot(location.xz, location.xz) / windData.x;
    vec3 windVelocity = vec3(0.0f, windData.y * exp(windArg), 0.0f);
    vec3 velocity = gravity.xyz + windVelocity + imageLoad(vortexVelocity, c).xyz;

    imageStore(source, c, vec4(velocity.xyz, density));
}
