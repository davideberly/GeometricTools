// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Shader.h>

namespace gte
{
    class ComputeProgram
    {
    public:
        // DX11 uses the class as is.  GL46 derives from the class to store
        // the shader and program handles.
        virtual ~ComputeProgram() = default;
        ComputeProgram() = default;

        // Member access.
        inline std::shared_ptr<Shader> const& GetComputeShader() const
        {
            return mCShader;
        }

        inline void SetComputeShader(std::shared_ptr<Shader> const& shader)
        {
            if (shader)
            {
                LogAssert(shader->GetType() == GT_COMPUTE_SHADER, "The input must be a compute shader.");
            }
            mCShader = shader;
        }

    private:
        std::shared_ptr<Shader> mCShader;
    };
}

