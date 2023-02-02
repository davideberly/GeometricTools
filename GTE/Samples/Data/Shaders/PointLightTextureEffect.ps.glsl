// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

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
    vec4 lightModelPosition;
    vec4 cameraModelPosition;
};

uniform sampler2D baseSampler;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

vec4 lit(float NdotL, float NdotH, float m)
{
  float ambient = 1.0;
  float diffuse = max(NdotL, 0.0);
  float specular = step(0.0, NdotL) * max(pow(NdotH, m), 0.0);
  return vec4(ambient, diffuse, specular, 1.0);
}

void main()
{
    vec3 modelLightDiff = vertexPosition - lightModelPosition.xyz;
    vec3 vertexDirection = normalize(modelLightDiff);
    float NDotL = -dot(vertexNormal, vertexDirection);
    vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);
    vec3 halfVector = normalize(viewVector - vertexDirection);
    float NDotH = dot(vertexNormal, halfVector);
    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);
    vec3 lightingColor = materialAmbient.rgb * lightingAmbient.rgb +
        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;

    float distance = length(modelLightDiff);
    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
        (lightingAttenuation.y + distance * lightingAttenuation.z));

    vec4 textureColor = texture(baseSampler, vertexTCoord);

    vec3 color = lightingColor * textureColor.rgb;
    pixelColor.rgb = materialEmissive.rgb + attenuation * color;
    pixelColor.a = materialDiffuse.a * textureColor.a;
}
