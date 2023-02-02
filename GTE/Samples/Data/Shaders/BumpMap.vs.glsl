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

// The BumpMaps sample vertex buffer has a vec3 normal channel, but
// the shader does not use it.  The location for that channel turns
// out to be 1, so modelLightDirection must have location 2,
// modelBaseTCoord must have location 3, and modelNormalTCoord must
// have location 4.
layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec3 modelNormal;  // unused
layout(location = 2) in vec3 modelLightDirection;
layout(location = 3) in vec2 modelBaseTCoord;
layout(location = 4) in vec2 modelNormalTCoord;
layout(location = 0) out vec3 vertexLightDirection;
layout(location = 1) out vec2 vertexBaseTCoord;
layout(location = 2) out vec2 vertexNormalTCoord;

void main()
{
    vertexLightDirection = modelLightDirection;
    vertexBaseTCoord = modelBaseTCoord;
    vertexNormalTCoord = modelNormalTCoord;
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
#else
    gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
#endif
}
