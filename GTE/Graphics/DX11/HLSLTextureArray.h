// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/DX11/HLSLResource.h>

namespace gte
{
    class HLSLTextureArray : public HLSLResource
    {
    public:
        // Construction and destruction.
        virtual ~HLSLTextureArray() = default;

        HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc);
        HLSLTextureArray(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index);

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

