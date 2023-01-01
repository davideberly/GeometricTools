// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/HLSLResource.h>

namespace gte
{
    class HLSLTexture : public HLSLResource
    {
    public:
        // Construction and destruction.
        virtual ~HLSLTexture() = default;

        HLSLTexture(D3D_SHADER_INPUT_BIND_DESC const& desc);
        HLSLTexture(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index);

        // Member access.
        inline uint32_t GetNumComponents() const
        {
            return mNumComponents;
        }

        inline uint32_t GetNumDimensions() const
        {
            return mNumDimensions;
        }

        inline bool IsGpuWritable() const
        {
            return mGpuWritable;
        }

    private:
        void Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc);

        uint32_t mNumComponents;
        uint32_t mNumDimensions;
        bool mGpuWritable;
    };
}
