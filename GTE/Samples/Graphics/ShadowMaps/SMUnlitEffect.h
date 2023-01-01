// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.04.02

#pragma once

#include <Graphics/GraphicsEngine.h>
using namespace gte;

class SMUnlitEffect : public VisualEffect
{
public:
    struct Geometry
    {
        Geometry()
            :
            worldMatrix(Matrix4x4<float>::Identity()),
            lightPVMatrix(Matrix4x4<float>::Identity())
        {
        }

        Matrix4x4<float> worldMatrix;
        Matrix4x4<float> lightPVMatrix;
    };

    struct Screen
    {
        Screen()
            :
            value{ 0.0f, 0.0f, 0.0f, 0.0f }
        {
        }

        std::array<float, 4> value;
    };

    SMUnlitEffect(
        std::shared_ptr<ProgramFactory> const& factory,
        std::string const& vsPath,
        std::string const& psPath,
        Geometry const& geometry,
        Screen const& screen,
        std::shared_ptr<Texture2> const& shadowTexture);

    inline std::shared_ptr<ConstantBuffer> const& GetGeometryBuffer() const
    {
        return mGeometryBuffer;
    }

    inline std::shared_ptr<ConstantBuffer> const& GetScreenBuffer() const
    {
        return mScreenBuffer;
    }

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mGeometryBuffer;

    // Pixel shader parameters.
    std::shared_ptr<ConstantBuffer> mScreenBuffer;
    std::shared_ptr<Texture2> mShadowTexture;
    std::shared_ptr<SamplerState> mSampler;
};
