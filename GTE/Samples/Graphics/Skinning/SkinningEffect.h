// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.28

#pragma once

#include <Graphics/VisualEffect.h>

namespace gte
{
    class SkinningEffect : public VisualEffect
    {
    public:
        SkinningEffect(std::shared_ptr<ProgramFactory> const& factory);

        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer) override;

        inline std::shared_ptr<ConstantBuffer> const& GetSkinningMatricesConstant() const
        {
            return mSkinningMatricesConstant;
        }

    private:
        // Vertex shader parameter.
        std::shared_ptr<ConstantBuffer> mSkinningMatricesConstant;

        // Shader source code as strings.
        static std::string const msGLSLVSSource;
        static std::string const msGLSLPSSource;
        static std::string const msHLSLVSSource;
        static std::string const msHLSLPSSource;
        static ProgramSources const msVSSource;
        static ProgramSources const msPSSource;
    };
}
