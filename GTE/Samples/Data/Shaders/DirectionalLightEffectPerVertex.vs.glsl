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

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec3 modelNormal;
layout(location = 0) out vec4 vertexColor;

vec4 lit(float NdotL, float NdotH, float m)
{
  float ambient = 1.0;
  float diffuse = max(NdotL, 0.0);
  float specular = step(0.0, NdotL) * max(pow(NdotH, m), 0.0);
  return vec4(ambient, diffuse, specular, 1.0);
}

void main()
{
    float NDotL = -dot(modelNormal, lightModelDirection.xyz);
    vec3 viewVector = normalize(cameraModelPosition.xyz - modelPosition);
    vec3 halfVector = normalize(viewVector - lightModelDirection.xyz);
    float NDotH = dot(modelNormal, halfVector);
    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

    vec3 color = materialAmbient.rgb * lightingAmbient.rgb +
        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

    vertexColor.rgb = materialEmissive.rgb + lightingAttenuation.w * color;
    vertexColor.a = materialDiffuse.a;
#if GTE_USE_MAT_VEC
    gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);
#else
    gl_Position = vec4(modelPosition, 1.0f) * pvwMatrix;
#endif
}
