// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Texture2.h>
#include <Graphics/VisualEffect.h>

// The DX11/HLSL engine properly maps channels of a position-normal-tcoord
// vertex to the inputs of Texture2Effect vertex shaders (using semantics).
// The GL4/GLSL engine does not properly map the channels because the
// 'location' for texture coordinates is listed as 1 in the GLSL code.  For
// now, this class is a patch to allow the GL4 version of Castle to display
// the textures correctly.

namespace gte
{
    class TexturePNT1Effect : public VisualEffect
    {
    public:
        // Construction.
        TexturePNT1Effect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& texture,
            SamplerState::Filter filter, SamplerState::Mode mode0, SamplerState::Mode mode1);

        // Member access.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer) override;

        inline std::shared_ptr<Texture2> const& GetTexture() const
        {
            return mTexture;
        }

        inline std::shared_ptr<SamplerState> const& GetSampler() const
        {
            return mSampler;
        }

    private:
        // Pixel shader parameters.
        std::shared_ptr<Texture2> mTexture;
        std::shared_ptr<SamplerState> mSampler;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources msVSSource;
        static ProgramSources msPSSource;
    };
}
