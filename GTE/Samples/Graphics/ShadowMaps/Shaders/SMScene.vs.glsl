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
    vec4 lightWorldPosition;
    vec4 cameraWorldPosition;
};

const mat4 lightBSMatrix = mat4
(
    vec4(0.5f, 0.0f, 0.0f, 0.0f),
    vec4(0.0f, 0.5f, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, 1.0f, 0.0f),
    vec4(0.5f, 0.5f, 0.0f, 1.0f)
);

const mat4 screenBSMatrix = mat4
(
    vec4(0.5f, 0.0f, 0.0f, 0.0f),
    vec4(0.0f, 0.5f, 0.0f, 0.0f),
    vec4(0.0f, 0.0f, 1.0f, 0.0f),
    vec4(0.5f, 0.5f, 0.0f, 1.0f)
);

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec3 modelNormal;
layout(location = 2) in vec2 modelTCoord;
layout(location = 0) out vec2 vertexTCoord;
layout(location = 1) out vec4 projTCoord;
layout(location = 2) out vec4 screenTCoord;
layout(location = 3) out vec3 vertexNormal;
layout(location = 4) out vec3 lightVector;
layout(location = 5) out vec3 eyeVector;

void main()
{
    // Transform the position from model space to clip space.
    vec4 hModelPosition = vec4(modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    vec4 pvwPosition = pvwMatrix * hModelPosition;
#else
    vec4 pvwPosition = hModelPosition * pvwMatrix;
#endif
    gl_Position = pvwPosition;

    // Pass through the texture coordinates.
    vertexTCoord = modelTCoord;

    // Transform the position from model space to world space.
#if GTE_USE_MAT_VEC
    vec4 worldPosition = worldMatrix * hModelPosition;
#else
    vec4 worldPosition = hModelPosition * worldMatrix;
#endif

    // Transform the normal from model space to world space.
    vec4 hModelNormal = vec4(modelNormal, 0.0f);
#if GTE_USE_MAT_VEC
    vec4 vertexNormal4 = worldMatrix * hModelNormal;
#else
    vec4 vertexNormal4 = hModelNormal * worldMatrix;
#endif
    vertexNormal = vertexNormal4.xyz;

    // Compute the projected texture coordinates.
#if GTE_USE_MAT_VEC
    vec4 lightSpacePosition = lightPVMatrix * worldPosition;
    projTCoord = lightBSMatrix * lightSpacePosition;
#else
    vec4 lightSpacePosition = worldPosition * lightPVMatrix;
    projTCoord = lightSpacePosition * lightBSMatrix;
#endif

    // Compute the screen-space texture coordinates.
#if GTE_USE_MAT_VEC
    screenTCoord = screenBSMatrix * pvwPosition;
#else
    screenTCoord = pvwPosition * screenBSMatrix;
#endif

    // Transform the light vector to tangent space.
    lightVector = lightWorldPosition.xyz - worldPosition.xyz;

    // Transform the eye vector into tangent space.
    eyeVector = cameraWorldPosition.xyz - worldPosition.xyz;
}
