// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

uniform PVWMatrix
{
    mat4 pvwMatrix;
};

uniform Geometry
{
    mat4 worldMatrix;
    mat4 lightPVMatrix;
};

const mat4 lightBSMatrix = mat4
(
    vec4(0.5f, 0.0f, 0.0f, 0.0f),
    vec4(0.0f, 0.5f, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, 1.0f, 0.0f),
    vec4(0.5f, 0.5f, 0.0f, 1.0f)
);

layout(location = 0) in vec3 modelPosition;
layout(location = 0) out vec4 projTCoord;
layout(location = 1) out float depth;

void main()
{
    // Transform the position from model space to clip space.
    vec4 hModelPosition = vec4(modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * hModelPosition;
#else
    gl_Position = hModelPosition * pvwMatrix;
#endif

    // Transform the position from model space to light space.
#if GTE_USE_MAT_VEC
    vec4 worldPosition = worldMatrix * hModelPosition;
    vec4 lightSpacePosition = lightPVMatrix * worldPosition;
#else
    vec4 worldPosition = hModelPosition * worldMatrix;
    vec4 lightSpacePosition = worldPosition * lightPVMatrix;
#endif

    // Compute the projected texture coordinates.
#if GTE_USE_MAT_VEC
    projTCoord = lightBSMatrix * lightSpacePosition;
#else
    projTCoord = lightSpacePosition * lightBSMatrix;
#endif

    // Output the distance from the light source.
    depth = lightSpacePosition.z;
}
