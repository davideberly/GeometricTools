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

uniform FogColorDensity
{
    vec4 fogColorDensity;
};

layout(location = 0) in vec3 inModelPosition;
layout(location = 1) in vec2 inModelBaseTCoord;
layout(location = 2) in vec2 inModelDetailTCoord;

layout(location = 0) out vec2 vertexBaseTCoord;
layout(location = 1) out vec2 vertexDetailTCoord;
layout(location = 2) out vec4 vertexFogInfo;

void main()
{
    // Transform the position from model space to clip space.
    vec4 modelPosition = vec4(inModelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    vec4 viewPosition = vwMatrix * modelPosition;
    gl_Position = pvwMatrix * modelPosition;
#else
    vec4 viewPosition = modelPosition * vwMatrix;;
    gl_Position = modelPosition * pvwMatrix;
#endif

    // Transform the position from model space to view space.  This is the
    // vector from the view-space eye position (the origin) to the view-space
    // vertex position.  The fog factor (vertexFogInfo.w) uses the
    // z-component of this vector, which is z-based depth, not range-based
    // depth.
    float fogSqrDistance = dot(viewPosition.xyz, viewPosition.xyz);
    float fogExpArg = -fogColorDensity.w * fogColorDensity.w * fogSqrDistance;
    vertexFogInfo.rgb = fogColorDensity.rgb;
    vertexFogInfo.w = exp(fogExpArg);

    // Pass through the texture coordinates.
    vertexBaseTCoord = inModelBaseTCoord;
    vertexDetailTCoord = inModelDetailTCoord;
}
