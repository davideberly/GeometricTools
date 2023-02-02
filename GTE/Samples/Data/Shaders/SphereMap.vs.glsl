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

uniform VWMatrix
{
    mat4 vwMatrix;
};

layout(location = 0) in vec3 inModelPosition;
layout(location = 1) in vec3 inModelNormal;

layout(location = 0) out vec2 vertexTCoord;

void main()
{
    vec4 modelPosition = vec4(inModelPosition, 1.0f);
    vec4 modelNormal = vec4(inModelNormal, 0.0f);

#if GTE_USE_MAT_VEC
    vec4 cameraSpacePosition = vwMatrix * modelPosition;
    vec3 cameraSpaceNormal = normalize((vwMatrix * modelNormal).xyz);
    gl_Position = pvwMatrix * modelPosition;
#else
    vec4 cameraSpacePosition = modelPosition * vwMatrix;
    vec3 cameraSpaceNormal = normalize((modelNormal * vwMatrix).xyz);
    gl_Position = modelPosition * pvwMatrix;
#endif

    vec3 eyeDirection = normalize(cameraSpacePosition.xyz);
    vec3 r = reflect(eyeDirection, cameraSpaceNormal);

    float oneMRZ = 1.0f - r.z;
    float invLength = 1.0f / sqrt(r.x * r.x + r.y * r.y + oneMRZ * oneMRZ);
    vertexTCoord = 0.5f * (r.xy * invLength + 1.0f);
}
