// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform WireParameters
{
    vec4 meshColor;
    vec4 edgeColor;
    vec2 windowSize;
};

const vec3 basis[3] =
{
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(0.0f, 0.0f, 1.0f)
};

layout(location = 0) in vec4 vertexColor[];
layout(location = 0) out vec4 pixelColor;
layout(location = 1) noperspective out vec3 edgeDistance;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
void main()
{
    vec2 pixel[3];
    float W[3];
    int i;
    for (i = 0; i < 3; ++i)
    {
        vec2 ndc = gl_in[i].gl_Position.xy / gl_in[i].gl_Position.w;
        pixel[i] = 0.5f * windowSize * (ndc + 1.0f);
    }

    int j0[3] = { 2, 0, 1 }, j1[3] = { 1, 2, 0 };
    for (i = 0; i < 3; ++i)
    {
        vec2 diff = pixel[i] - pixel[j1[i]];
        vec2 edge = pixel[j0[i]] - pixel[j1[i]];
        float edgeLength = length(edge);
        float distance;
        if (edgeLength > 0.0f)
        {
            distance = abs(dot(diff, vec2(edge.y, -edge.x)) / edgeLength);
        }
        else
        {
            distance = 0.0f;
        }

        gl_Position = gl_in[i].gl_Position;
        pixelColor = vertexColor[i];
        edgeDistance = distance * basis[i];
        EmitVertex();
    }

    EndPrimitive();
}
