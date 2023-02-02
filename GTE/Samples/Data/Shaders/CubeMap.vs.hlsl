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

cbuffer WMatrix
{
    float4x4 wMatrix;
};

cbuffer CameraWorldPosition
{
    float4 cameraWorldPosition;
};

struct VS_INPUT
{
    float3 modelPosition : POSITION;
    float3 modelNormal : NORMAL;
    float4 modelColor : COLOR;
};

struct VS_OUTPUT
{
    float4 vertexColor : COLOR;
    float3 cubeTCoord : TEXCOORD0;
    float4 clipPosition : SV_POSITION;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 hModelPosition = float4(input.modelPosition, 1.0f);
    float3 worldPosition, worldNormal;
#if GTE_USE_MAT_VEC
    output.clipPosition = mul(pvwMatrix, hModelPosition);
    worldPosition = mul(wMatrix, hModelPosition).xyz;
    worldNormal = normalize(mul(wMatrix, float4(input.modelNormal, 0.0f))).xyz;
#else
    output.clipPosition = mul(hModelPosition, pvwMatrix);
    worldPosition = mul(hModelPosition, wMatrix).xyz;
    worldNormal = normalize(mul(float4(input.modelNormal, 0.0f), wMatrix)).xyz;
#endif

    // Calculate the eye direction.  The direction does not have to be
    // normalized, because the texture coordinates for the cube map are
    // invariant to scaling: directions V and s*V for s > 0 generate the
    // same texture coordinates.
    float3 eyeDirection = worldPosition - cameraWorldPosition.xyz;

    // Calculate the reflected vector.
    output.cubeTCoord = reflect(eyeDirection, worldNormal);

    // Pass through the model color.
    output.vertexColor = input.modelColor;

    return output;
}
