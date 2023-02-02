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

uniform Material
{
    vec4 materialEmissive;
    vec4 materialAmbient;
};

uniform Light
{
    vec4 lightingAmbient;
    vec4 lightingAttenuation;
};

layout(location = 0) in vec3 modelPosition;
layout(location = 0) out vec4 vertexColor;

void main()
{
    vec3 ambient = lightingAttenuation.w * lightingAmbient.rgb;
    vertexColor.rgb = materialEmissive.rgb + materialAmbient.rgb * ambient;
    vertexColor.a = 1.0f;
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
#else
    gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
#endif
}
