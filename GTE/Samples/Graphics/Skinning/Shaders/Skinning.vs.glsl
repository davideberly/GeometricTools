// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

uniform PVWMatrix
{
    mat4 pvwMatrix;
};

uniform SkinningMatrices
{
    mat4 skinningMatrix0;
    mat4 skinningMatrix1;
    mat4 skinningMatrix2;
    mat4 skinningMatrix3;
};

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec4 modelColor;
layout(location = 2) in vec4 modelWeights;
layout(location = 0) out vec4 vertexColor;

void main()
{
    // This shader has a fixed number of skinning matrices per vertex. If you
    // want a number that varies with the vertex, pass in an array of skinning
    // matrices. Also pass in texture coordinates that are used as lookups
    // into the array.

    // Calculate the position by adding together a convex combination of
    // transformed positions.
    vec4 hModelPosition = vec4(modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    vec4 position0 = (skinningMatrix0 * hModelPosition) * modelWeights.x;
    vec4 position1 = (skinningMatrix1 * hModelPosition) * modelWeights.y;
    vec4 position2 = (skinningMatrix2 * hModelPosition) * modelWeights.z;
    vec4 position3 = (skinningMatrix3 * hModelPosition) * modelWeights.w;
#else
    vec4 position0 = (hModelPosition * skinningMatrix0) * modelWeights.x;
    vec4 position1 = (hModelPosition * skinningMatrix1) * modelWeights.y;
    vec4 position2 = (hModelPosition * skinningMatrix2) * modelWeights.z;
    vec4 position3 = (hModelPosition * skinningMatrix3) * modelWeights.w;
#endif
    vec4 skinPosition = position0 + position1 + position2 + position3;

    // Transform the position from model space to clip space.
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * skinPosition;
#else
    gl_Position = skinPosition * pvwMatrix;
#endif

    // The vertex color is passed through.
    vertexColor = modelColor;
}
