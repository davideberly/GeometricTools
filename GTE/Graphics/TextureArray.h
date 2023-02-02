// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Texture.h>
#include <cstdint>

namespace gte
{
    class TextureArray : public Texture
    {
    protected:
        // Abstract base class (shim).  All items in the array have the same
        // format, number of dimensions, dimension values, and mipmap status.
        TextureArray(uint32_t numItems, uint32_t format,
            uint32_t numDimensions, uint32_t dim0, uint32_t dim1,
            uint32_t dim2, bool hasMipmaps, bool createStorage);

    public:
        // Mipmap information.
        inline uint32_t GetOffsetFor(uint32_t item, uint32_t level) const
        {
            return Texture::GetOffsetFor(item, level);
        }

        inline char const* GetDataFor(uint32_t item, uint32_t level) const
        {
            return Texture::GetDataFor(item, level);
        }

        inline char* GetDataFor(uint32_t item, uint32_t level)
        {
            return Texture::GetDataFor(item, level);
        }

        template <typename T>
        inline T const* GetFor(uint32_t item, uint32_t level) const
        {
            return Texture::GetFor<T>(item, level);
        }

        template <typename T>
        inline T* GetFor(uint32_t item, uint32_t level)
        {
            return Texture::GetFor<T>(item, level);
        }

    public:
        // For use by the Shader class for storing reflection information.
        static int32_t const shaderDataLookup = 5;

        // Used as face index in TextureCube and TextureCubeArray.
        static uint32_t constexpr cubeFacePositiveX = 0;
        static uint32_t constexpr cubeFaceNegativeX = 1;
        static uint32_t constexpr cubeFacePositiveY = 2;
        static uint32_t constexpr cubeFaceNegativeY = 3;
        static uint32_t constexpr cubeFacePositiveZ = 4;
        static uint32_t constexpr cubeFaceNegativeZ = 5;
        static uint32_t constexpr cubeFaceCount = 6;
    };

    typedef std::function<void(std::shared_ptr<TextureArray> const&)> TextureArrayUpdater;
    typedef std::function<void(std::shared_ptr<TextureArray> const&, uint32_t)> TextureArrayLevelUpdater;
}
