// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.03

cbuffer PVWMatrix
{
    float4x4 pvwMatrix;
};

cbuffer ProjectorMatrix
{
    float4x4 projectorMatrix;
};

cbuffer Material
{
    float4 materialEmissive;
    float4 materialAmbient;
    float4 materialDiffuse;
    float4 materialSpecular;
};

cbuffer Lighting
{
    float4 lightingAmbient;
    float4 lightingDiffuse;
    float4 lightingSpecular;
    float4 lightingAttenuation;
};

cbuffer LightCameraGeometry
{
    float4 lightModelDirection;
    float4 cameraModelPosition;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float3 modelNormal : NORMAL;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR;
    float4 projectorTCoord : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    float NDotL = -dot(input.modelNormal, lightModelDirection.xyz);
    float3 viewVector = normalize(cameraModelPosition.xyz - input.modelPosition);
    float3 halfVector = normalize(viewVector - lightModelDirection.xyz);
    float NDotH = dot(input.modelNormal, halfVector);
    float4 lighting = lit(NDotL, NDotH, materialSpecular.a);

    output.vertexColor.rgb = materialEmissive.rgb +
        materialAmbient.rgb * lightingAmbient.rgb +
        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +
        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;
    output.vertexColor.a = materialDiffuse.a;

    float4 modelPosition = float4(input.modelPosition, 1.0f);
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));
    output.projectorTCoord = mul(projectorMatrix, modelPosition);
#else
    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);
    output.projectorTCoord = mul(modelPosition, projectorMatrix);
#endif
    return output;
}
