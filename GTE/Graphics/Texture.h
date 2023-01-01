// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DataFormat.h>
#include <Graphics/Resource.h>
#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace gte
{
    class Texture : public Resource
    {
    protected:
        // Abstract base class for single textures and for texture arrays.
        // All items in a texture array have the same format, number of
        // dimensions, dimension values and mipmap status.
        Texture(uint32_t numItems, uint32_t format, uint32_t numDimensions,
            uint32_t dim0, uint32_t dim1, uint32_t dim2,
            bool hasMipmaps, bool createStorage);

    public:
        // Member access.
        inline uint32_t GetNumItems() const
        {
            return mNumItems;
        }

        inline uint32_t GetFormat() const
        {
            return mFormat;
        }

        inline uint32_t GetNumDimensions() const
        {
            return mNumDimensions;
        }

        inline uint32_t GetDimension(int32_t i) const
        {
            return mLDimension[0][i];
        }

        // Subresource information.
        struct Subresource
        {
            uint32_t item;
            uint32_t level;
            char* data;
            uint32_t rowPitch;
            uint32_t slicePitch;
        };

        // Mipmap information.
        enum { MAX_MIPMAP_LEVELS = 16 };

        inline bool HasMipmaps() const
        {
            return mHasMipmaps;
        }

        inline uint32_t GetNumLevels() const
        {
            return mNumLevels;
        }

        inline uint32_t GetDimensionFor(uint32_t level, int32_t i) const
        {
            return mLDimension[level][i];
        }

        inline uint32_t GetNumElementsFor(uint32_t level) const
        {
            return mLNumBytes[level] / mElementSize;
        }

        inline uint32_t GetNumBytesFor(uint32_t level) const
        {
            return mLNumBytes[level];
        }

        inline uint32_t GetOffsetFor(uint32_t item, uint32_t level) const
        {
            return mLOffset[item][level];
        }

        inline char const* GetDataFor(uint32_t item, uint32_t level) const
        {
            return mData ? (mData + mLOffset[item][level]) : nullptr;
        }

        inline char* GetDataFor(uint32_t item, uint32_t level)
        {
            return mData ? (mData + mLOffset[item][level]) : nullptr;
        }

        template <typename T>
        inline T const* GetFor(uint32_t item, uint32_t level) const
        {
            return reinterpret_cast<T const*>(GetDataFor(item, level));
        }

        template <typename T>
        inline T* GetFor(uint32_t item, uint32_t level)
        {
            return reinterpret_cast<T*>(GetDataFor(item, level));
        }

        // Subresource indexing:  index = numLevels*item + level
        inline uint32_t GetNumSubresources() const
        {
            return mNumItems * mNumLevels;
        }

        uint32_t GetIndex(uint32_t item, uint32_t level) const;

        Subresource GetSubresource(uint32_t index) const;

        // Request that the GPU compute mipmap levels when the base-level texture
        // data is modified.  The AutogenerateMipmaps call should be made before
        // binding the texture to the engine.  If the texture does not have mipmaps,
        // the AutogenerateMipmaps call will not set mAutogenerateMipmaps to true.
        void AutogenerateMipmaps();

        inline bool WantAutogenerateMipmaps() const
        {
            return mAutogenerateMipmaps;
        }

    protected:
        // Support for computing the numElements parameter for the Resource
        // constructor.  This is necessary when mipmaps are requested.
        static uint32_t GetTotalElements(uint32_t numItems,
            uint32_t dim0, uint32_t dim1, uint32_t dim2,
            bool hasMipmaps);

        uint32_t mNumItems;
        uint32_t mFormat;
        uint32_t mNumDimensions;
        uint32_t mNumLevels;
        std::array<std::array<uint32_t, 3>, MAX_MIPMAP_LEVELS> mLDimension;
        std::array<uint32_t, MAX_MIPMAP_LEVELS> mLNumBytes;
        std::vector<std::array<uint32_t, MAX_MIPMAP_LEVELS>> mLOffset;
        bool mHasMipmaps;
        bool mAutogenerateMipmaps;
    };

    typedef std::function<void(std::shared_ptr<Texture> const&)> TextureUpdater;
    typedef std::function<void(std::shared_ptr<Texture> const&, uint32_t)> TextureLevelUpdater;
}
