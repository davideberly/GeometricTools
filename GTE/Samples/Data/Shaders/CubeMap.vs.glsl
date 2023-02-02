// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform PVWMatrix
{
    mat4 pvwMatrix;
};

uniform WMatrix
{
    mat4 wMatrix;
};

uniform CameraWorldPosition
{
    vec4 cameraWorldPosition;
};

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec3 modelNormal;
layout(location = 2) in vec4 modelColor;

layout(location = 0) out vec4 vertexColor;
layout(location = 1) out vec3 cubeTCoord;

void main()
{
    vec4 hModelPosition = vec4(modelPosition, 1.0f);
    vec3 worldPosition, worldNormal;
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * hModelPosition;
    worldPosition = (wMatrix * hModelPosition).xyz;
    worldNormal = normalize(wMatrix * vec4(modelNormal, 0.0f)).xyz;
#else
    gl_Position = hModelPosition * pvwMatrix;
    worldPosition = (hModelPosition * wMatrix).xyz;
    worldNormal = normalize(vec4(modelNormal, 0.0f) * wMatrix).xyz;
#endif

    // Calculate the eye direction.  The direction does not have to be
    // normalized, because the texture coordinates for the cube map are
    // invariant to scaling: directions V and s*V for s > 0 generate the
    // same texture coordinates.
    vec3 eyeDirection = worldPosition - cameraWorldPosition.xyz;

    // Calculate the reflected vector.
    cubeTCoord = reflect(eyeDirection, worldNormal);

    // Pass through the model color.
    vertexColor = modelColor;
}
