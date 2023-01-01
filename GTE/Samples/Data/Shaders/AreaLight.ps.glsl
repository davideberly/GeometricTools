// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

uniform Material
{
    // Properties of the surface.
    vec4 materialEmissive;
    vec4 materialAmbient;
    vec4 materialDiffuse;
    vec4 materialSpecular;
};

uniform Camera
{
    vec4 cameraModelPosition;
};

uniform AreaLight
{
    vec4 lightingAmbient;
    vec4 lightingDiffuse;
    vec4 lightingSpecular;
    vec4 lightingAttenuation;

    // The light is rectangular in shape, represented by an oriented bounding
    // rectangle in 3D.  The application is responsible for setting the
    // rectangle members in the model space of the surface to be illuminated.
    vec4 rectPosition;  // (x,y,z,1)
    vec4 rectNormal, rectAxis0, rectAxis1;  // (x,y,z,0)
    vec4 rectExtent;  // (extent0, extent1, *, *)
};

uniform sampler2D baseSampler;
uniform sampler2D normalSampler;

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec2 vertexTCoord;
layout(location = 0) out vec4 pixelColor;

vec4 ComputeLightModelPosition(in vec4 vertexPosition)
{
    vec4 diff = vertexPosition - rectPosition;
    vec2 crd = vec2(dot(rectAxis0, diff), dot(rectAxis1, diff));
    crd = max(min(crd, rectExtent.xy), -rectExtent.xy);
    vec4 closest = rectPosition + crd.x * rectAxis0 + crd.y * rectAxis1;
    return closest;
}

vec4 lit(float NdotL, float NdotH, float m)
{
    float ambient = 1.0f;
    float diffuse = max(NdotL, 0.0f);
    float specular = step(0.0f, NdotL) * pow(max(NdotH, 0.0f), m);
    return vec4(ambient, diffuse, specular, 1.0f);
}

void main()
{
    vec4 lightModelPosition = ComputeLightModelPosition(vertexPosition);
    vec3 modelLightDiff = vertexPosition.xyz - lightModelPosition.xyz;
    float distance = length(modelLightDiff);
    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *
        (lightingAttenuation.y + distance * lightingAttenuation.z));

    // This code is specific to the brick texture used by the application.  The
    // brick geometry is in a plane whose normal is consistent with the normal
    // map generated for the brick texture.  Thus, the normal may be looked up
    // directly without computing tangent-space quantities.
    vec3 vertexNormal = texture(normalSampler, vertexTCoord).rgb;
    vertexNormal = normalize(2.0f * vertexNormal - 1.0f);

    vec3 vertexDirection = normalize(modelLightDiff);
    float NDotL = -dot(vertexNormal, vertexDirection);
    vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);
    vec3 halfVector = normalize(viewVector - vertexDirection);
    float NDotH = dot(vertexNormal, halfVector);
    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);

    vec3 emissive = materialEmissive.rgb;
    vec3 ambient = materialAmbient.rgb * lightingAmbient.rgb;
    vec4 textureDiffuse = texture(baseSampler, vertexTCoord);
    vec3 diffuse = materialDiffuse.rgb * textureDiffuse.rgb * lightingDiffuse.rgb;
    vec3 specular = materialSpecular.rgb * lightingSpecular.rgb;

    vec3 colorRGB = emissive +
        attenuation * (ambient + lighting.y * diffuse + lighting.z * specular);
    float colorA = materialDiffuse.a * textureDiffuse.a;
    pixelColor = vec4(colorRGB, colorA);
}
