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

uniform Vortex
{
    vec4 data;      // (x, y, variance, amplitude)
};

layout(rg32f) uniform readonly image2D inVelocity;
layout(rg32f) uniform writeonly image2D outVelocity;

layout (local_size_x = NUM_X_THREADS, local_size_y = NUM_Y_THREADS, local_size_z = 1) in;
void main()
{
    ivec2 c = ivec2(gl_GlobalInvocationID.xy);
    vec2 location = spaceDelta.xy * (c + 0.5f);
    vec2 diff = location - data.xy;
    float arg = -dot(diff, diff) / data.z;
    float magnitude = data.w * exp(arg);
    vec2 vortexVelocity = magnitude * vec2(diff.y, -diff.x);
    imageStore(outVelocity, c, vec4(imageLoad(inVelocity, c).xy + vortexVelocity, 0.0f, 0.0f));
};
