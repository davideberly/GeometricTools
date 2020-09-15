// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2020
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/VisualEffect.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/SamplerState.h>
#include <Graphics/Texture2.h>
#include <Mathematics/Vector4.h>

namespace gte
{
    class TextEffect : public VisualEffect
    {
    public:
        // Construction.
        TextEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& texture);

        // Support for typesetting.
        inline std::shared_ptr<ConstantBuffer> const& GetTranslate() const
        {
            return mTranslate;
        }

        inline std::shared_ptr<ConstantBuffer> const& GetColor() const
        {
            return mColor;
        }

        void SetTranslate(float x, float y);
        void SetColor(Vector4<float> const& color);

    private:
        std::shared_ptr<ConstantBuffer> mTranslate;
        std::shared_ptr<ConstantBuffer> mColor;
        std::shared_ptr<SamplerState> mSamplerState;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
