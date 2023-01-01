// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

vec4 lit(float NdotL, float NdotH, float m)
{
    float ambient = 1.0;
    float diffuse = max(NdotL, 0.0);
    float specular = step(0.0, NdotL) * max(pow(NdotH, m), 0.0);
    return vec4(ambient, diffuse, specular, 1.0);
}

uniform PVWMatrix
{
    mat4 pvwMatrix;
};

uniform ProjectorMatrix
{
    mat4 projectorMatrix;
};

uniform Material
{
    vec4 materialEmissive;
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
};

uniform Lighting
{
    vec4 lightingAmbient;
    vec4 lightingDiffuse;
    vec4 lightingSpecular;
    vec4 lightingAttenuation;
};

uniform LightCameraGeometry
{
    vec4 lightModelDirection;
    vec4 cameraModelPosition;
};

layout(location = 0) in vec3 inModelPosition;
layout(location = 1) in vec3 inModelNormal;

layout(location = 0) out vec4 vertexColor;
layout(location = 1) out vec4 projectorTCoord;

void main()
{
    float NDotL = -dot(inModelNormal, lightModelDirection.xyz);
    vec3 viewVector = normalize(cameraModelPosition.xyz - inModelPosition);
    vec3 halfVector = normalize(viewVector - lightModelDirection.xyz);
    float NDotH = dot(inModelNormal, halfVector);
    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

    vertexColor.rgb = materialEmissive.rgb +
        materialAmbient.rgb * lightingAmbient.rgb +
        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    vertexColor.a = materialDiffuse.a;

    vec4 modelPosition = vec4(inModelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * modelPosition;
    projectorTCoord = projectorMatrix * modelPosition;
#else
    gl_Position = modelPosition * pvwMatrix;
    projectorTCoord = modelPosition * projectorMatrix;
#endif
};
