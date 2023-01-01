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

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec2 modelTCoord;

layout(location = 0) out vec2 vertexTCoord;
layout(location = 1) noperspective out float perspectiveDepth;

void main()
{
#if GTE_USE_MAT_VEC
    vec4 clipPosition = pvwMatrix * vec4(modelPosition, 1.0f);
#else
    vec4 clipPosition = vec4(modelPosition, 1.0f) * pvwMatrix;
#endif
    clipPosition.y = -clipPosition.y;
    gl_Position = clipPosition;
    vertexTCoord = modelTCoord;
    perspectiveDepth = clipPosition.z / clipPosition.w;
}
