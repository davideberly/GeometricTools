// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#pragma once

#include <Graphics/GraphicsEngine.h>
using namespace gte;

class SMSceneEffect : public VisualEffect
{
public:
    struct Geometry
    {
        Geometry()
            :
            worldMatrix(Matrix4x4<float>::Identity()),
            lightPVMatrix(Matrix4x4<float>::Identity()),
            lightWorldPosition(Vector4<float>::Unit(3)),
            cameraWorldPosition(Vector4<float>::Unit(3))
        {
        }

        Matrix4x4<float> worldMatrix;
        Matrix4x4<float> lightPVMatrix;
        Vector4<float> lightWorldPosition;
        Vector4<float> cameraWorldPosition;
    };

    struct LightColor
    {
        LightColor()
            :
            color(Vector4<float>::Zero())
        {
        }

        Vector4<float> color;
    };

    SMSceneEffect(
        std::shared_ptr<ProgramFactory> const& factory,
        std::string const& vsPath,
        std::string const& psPath,
        Geometry const& geometry,
        LightColor const& lightColor,
        std::shared_ptr<Texture2> const& baseTexture,
        std::shared_ptr<Texture2> const& blurTexture,
        std::shared_ptr<Texture2> const& projTexture);

    inline std::shared_ptr<ConstantBuffer> const& GetGeometryBuffer() const
    {
        return mGeometryBuffer;
    }

    inline std::shared_ptr<ConstantBuffer> const& GetLightColorBuffer() const
    {
        return mLightColorBuffer;
    }

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mGeometryBuffer;

    // Pixel shader parameters.
    std::shared_ptr<ConstantBuffer> mLightColorBuffer;
    std::shared_ptr<Texture2> mBaseTexture;
    std::shared_ptr<Texture2> mBlurTexture;
    std::shared_ptr<Texture2> mProjTexture;
    std::shared_ptr<SamplerState> mSampler;
};
