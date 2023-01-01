// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/HLSLResource.h>
#include <cstdint>
#include <vector>

namespace gte
{
    class HLSLShaderVariable
    {
    public:
        struct Description
        {
            Description()
                :
                name(""),
                offset(0),
                numBytes(0),
                flags(0),
                textureStart(0),
                textureNumSlots(0),
                samplerStart(0),
                samplerNumSlots(0),
                defaultValue{}
            {
            }

            std::string name;
            uint32_t offset;
            uint32_t numBytes;
            uint32_t flags;
            uint32_t textureStart;
            uint32_t textureNumSlots;
            uint32_t samplerStart;
            uint32_t samplerNumSlots;
            std::vector<uint8_t> defaultValue;
        };

        // Construction.  Shader variables are reported for constant buffers,
        // texture buffers, and structs defined in the shaders (resource
        // binding information).
        HLSLShaderVariable() = default;

        // Deferred construction for shader reflection.  This function is
        // intended to be write-once.
        void SetDescription(D3D_SHADER_VARIABLE_DESC const& desc);

        // Member access.
        inline std::string const& GetName() const
        {
            return mDesc.name;
        }

        inline uint32_t GetOffset() const
        {
            return mDesc.offset;
        }

        inline uint32_t GetNumBytes() const
        {
            return mDesc.numBytes;
        }

        inline uint32_t GetFlags() const
        {
            return mDesc.flags;
        }

        inline uint32_t GetTextureStart() const
        {
            return mDesc.textureStart;
        }

        inline uint32_t GetTextureNumSlots() const
        {
            return mDesc.textureNumSlots;
        }

        inline uint32_t GetSamplerStart() const
        {
            return mDesc.samplerStart;
        }

        inline uint32_t GetSamplerNumSlots() const
        {
            return mDesc.samplerNumSlots;
        }

        inline std::vector<uint8_t> const& GetDefaultValue() const
        {
            return mDesc.defaultValue;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;
    };
}
