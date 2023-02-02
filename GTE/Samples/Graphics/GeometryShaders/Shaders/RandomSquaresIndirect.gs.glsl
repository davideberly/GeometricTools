// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

const vec4 offset[4] =
{
    vec4(-1.0f, -1.0f, 0.0f, 0.0f), // left bottom
    vec4(+1.0f, -1.0f, 0.0f, 0.0f), // right bottom
    vec4(-1.0f, +1.0f, 0.0f, 0.0f), // left top
    vec4(+1.0f, +1.0f, 0.0f, 0.0f)  // right top
};

uniform Matrices
{
    mat4 vwMatrix;
    mat4 pMatrix;
};

in Vertex
{
    int id;
} vertex[];

struct Particle
{
    vec3 position;
    vec3 color;
    float size;
};

buffer particles
{
    Particle data[];
} particlesSB;

layout(location = 0) out vec3 pixelColor;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
void main()
{
    Particle particle = particlesSB.data[vertex[0].id];

#if GTE_USE_MAT_VEC
    vec4 viewPosition = vwMatrix * vec4(particle.position, 1.0f);
#else
    vec4 viewPosition = vec4(particle.position, 1.0f) * vwMatrix;
#endif

    for (int i = 0; i < 4; ++i)
    {
        vec4 corner = viewPosition + particle.size*offset[i];
#if GTE_USE_MAT_VEC
        gl_Position = pMatrix * corner;
#else
        gl_Position = corner * pMatrix;
#endif
        pixelColor = particle.color;
        EmitVertex();
    }
    EndPrimitive();
}
