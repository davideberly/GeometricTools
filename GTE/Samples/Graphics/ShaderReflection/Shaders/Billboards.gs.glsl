// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

in VS_STRUCT
{
    vec3 position;
    vec3 color;
    float size;
} vertex[];

out GS_OUTPUT
{
    vec3 color;
} gvertex;

uniform Matrices
{
    mat4 vwMatrix;
    mat4 pMatrix;
};

const vec4 offset[4] =
{
    vec4(-1.0f, -1.0f, 0.0f, 0.0f),
    vec4(+1.0f, -1.0f, 0.0f, 0.0f),
    vec4(-1.0f, +1.0f, 0.0f, 0.0f),
    vec4(+1.0f, +1.0f, 0.0f, 0.0f)
};

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;
void main()
{
    vec3 outColor[4];
    vec4 outPosition[4];
    vec4 viewPosition = vwMatrix * vec4(vertex[0].position, 1.0f);
    int i;
    for (i = 0; i < 4; ++i)
    {
        vec4 corner = viewPosition + vertex[0].size*offset[i];
        outPosition[i] = pMatrix * corner;
        outColor[i] = vertex[0].color;
    }

    gl_Position = outPosition[0];
    gvertex.color = outColor[0];
    EmitVertex();
    gl_Position = outPosition[1];
    gvertex.color = outColor[1];
    EmitVertex();
    gl_Position = outPosition[3];
    gvertex.color = outColor[3];
    EmitVertex();
    EndPrimitive();

    gl_Position = outPosition[0];
    gvertex.color = outColor[0];
    EmitVertex();
    gl_Position = outPosition[3];
    gvertex.color = outColor[3];
    EmitVertex();
    gl_Position = outPosition[2];
    gvertex.color = outColor[2];
    EmitVertex();
    EndPrimitive();
}
