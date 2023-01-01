// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

layout(std140)
uniform PowerFactor
{
    float powerFactor;
};

uniform sampler2D grassSampler;
uniform sampler2D stoneSampler;
uniform sampler2D cloudSampler;
uniform sampler1D blendSampler;

layout(location = 0) in vec2 vertexGroundTCoord;
layout(location = 1) in float vertexBlendTCoord;
layout(location = 2) in vec2 vertexCloudTCoord;
layout(location = 3) in vec2 vertexFlowDirection;

layout(location = 0) out vec4 pixelColor;

void main()
{
    vec4 grassColor = texture(grassSampler, vertexGroundTCoord);
    vec4 stoneColor = texture(stoneSampler, vertexGroundTCoord);
    vec4 blendValue = texture(blendSampler, vertexBlendTCoord);

    vec2 offsetCloudTCoord = vertexCloudTCoord + vertexFlowDirection;    
    vec4 cloudColor = texture(cloudSampler, offsetCloudTCoord);

    float stoneWeight = pow(abs(blendValue.r), powerFactor);
    float grassWeight = 1.0f - stoneWeight;

    pixelColor = cloudColor*(grassWeight * grassColor + stoneWeight * stoneColor);
}
