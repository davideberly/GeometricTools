// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

uniform LightColor
{
    vec4 lightColor;
};

uniform sampler2D baseSampler;
uniform sampler2D blurSampler;
uniform sampler2D projSampler;

layout(location = 0) in vec2 vertexTCoord;
layout(location = 1) in vec4 projTCoord;
layout(location = 2) in vec4 screenTCoord;
layout(location = 3) in vec3 vertexNormal;
layout(location = 4) in vec3 lightVector;
layout(location = 5) in vec3 eyeVector;
layout(location = 0) out vec4 pixelColor;

void main()
{
    // Normalize the input vectors.
    vec3 vertexNormal0 = normalize(vertexNormal);
    vec3 lightVector0 = normalize(lightVector);
    vec3 eyeVector0 = normalize(eyeVector);

    // Get the base color.
    vec4 baseColor = texture(baseSampler, vertexTCoord);

    // Compute the ambient lighting term (zero, for this example).
    float ambient = 0.0f;

    // Compute the diffuse lighting term.
    float NdL = dot(vertexNormal0, lightVector0);
    float diffuse = max(NdL, 0.0f);

    // Compute the specular lighting term.
    float specular;
    if (diffuse != 0.0f)
    {
        vec3 tmp = 2.0f * NdL * vertexNormal0 - lightVector0;
        specular = pow(max(dot(tmp, lightVector0), 0.0f), 8.0f);
    }
    else
    {
        specular = 0.0f;
    }

    // Clamp the spot texture to a disk centered in the texture in order to
    // give the appearance of a spotlight cone.
    float u = projTCoord.x / projTCoord.w;
    float v = projTCoord.y / projTCoord.w;
    float du = u - 0.5f;
    float dv = v - 0.5f;
    float rsqr = du * du + dv * dv;
    float weight = (rsqr <= 0.25f ? 0.0f : 1.0f);

    vec2 blurTCoord = screenTCoord.xy / screenTCoord.w;
    float shadow = texture(blurSampler, blurTCoord).r;
    vec4 projColor = texture(projSampler, vec2(u, 1.0f - v));
    vec4 shadowColor = shadow * projColor;
    vec4 ambientColor = ambient * baseColor;
    vec4 diffuseColor = diffuse * baseColor * lightColor;
    vec4 specularColor = specular * baseColor * lightColor.a;

    pixelColor = ambientColor + (diffuseColor + specularColor) * (
        weight + (1.0f - weight) * shadowColor);

    pixelColor.a = 1.0f;
}
