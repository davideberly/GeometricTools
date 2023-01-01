// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Matrices
{
    mat4 vwMatrix;
    mat4 pMatrix;
};

const vec4 offset[4] =
{
    vec4(-1.0f, -1.0f, 0.0f, 0.0f), // left bottom
    vec4(+1.0f, -1.0f, 0.0f, 0.0f), // right bottom
    vec4(-1.0f, +1.0f, 0.0f, 0.0f), // left top
    vec4(+1.0f, +1.0f, 0.0f, 0.0f)  // right top
};

in Vertex { vec4 colorSize; } vertex[];
layout(location = 0) out vec3 pixelColor;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
void main()
{
    vec4 position = gl_in[0].gl_Position;
#if GTE_USE_MAT_VEC
    vec4 viewPosition = vwMatrix * position;
#else
    vec4 viewPosition = position * vwMatrix;
#endif

    for (int i = 0; i < 4; ++i)
    {
        vec4 corner = viewPosition + vertex[0].colorSize.a * offset[i];
#if GTE_USE_MAT_VEC
        gl_Position = pMatrix * corner;
#else
        gl_Position = corner * pMatrix;
#endif
        pixelColor = vertex[0].colorSize.rgb;
        EmitVertex();
    }
    EndPrimitive();
}
