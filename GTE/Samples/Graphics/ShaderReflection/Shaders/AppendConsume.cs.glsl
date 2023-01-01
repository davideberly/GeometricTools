// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

struct Particle
{
    vec3 position;
    vec3 color;
    float size;
};

// Equivalent in GLSL to:
// ConsumeStructuredBuffer<Particle> particlesIn;
//
buffer particlesIn { Particle data[]; } particlesInSB;
layout(binding=0, offset=0) uniform atomic_uint particlesInCounter;
Particle particlesInConsume()
{
    // atomicCounterDecrement returns value after decrement
    uint index = atomicCounterDecrement(particlesInCounter);

    return particlesInSB.data[index];
}

// Equivalent in GLSL to:
// AppendStructuredBuffer<Particle> particlesOut;
//
buffer particlesOut { Particle data[]; } particlesOutSB;
layout(binding=0, offset=4) uniform atomic_uint particlesOutCounter;
void particlesOutAppend(Particle p)
{
    // atomicCounterIncrement returns value before increment
    uint index = atomicCounterIncrement(particlesOutCounter);

    particlesOutSB.data[index] = p;
}

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    Particle p = particlesInConsume();

    particlesOutAppend(p);
}
