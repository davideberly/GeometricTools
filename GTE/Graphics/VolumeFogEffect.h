// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.22

#pragma once

#include <Graphics/VisualEffect.h>

namespace gte
{
    class VolumeFogEffect : public VisualEffect
    {
    public:
        // Construction.
        VolumeFogEffect(std::shared_ptr<ProgramFactory> const& factory,
            std::shared_ptr<Texture2> const& texture,
            SamplerState::Filter filter, SamplerState::Mode mode0,
            SamplerState::Mode mode1);

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
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
