// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Mathematics/BitHacks.h>
#include <Graphics/Texture.h>
using namespace gte;

Texture::Texture(uint32_t numItems, uint32_t format, uint32_t numDimensions,
    uint32_t dim0, uint32_t dim1, uint32_t dim2, bool hasMipmaps,
    bool createStorage)
    :
    Resource(GetTotalElements(numItems, dim0, dim1, dim2, hasMipmaps),
        DataFormat::GetNumBytesPerStruct(format), createStorage),
    mNumItems(numItems),
    mFormat(format),
    mNumDimensions(numDimensions),
    mNumLevels(1),
    mLOffset(numItems),
    mHasMipmaps(hasMipmaps),
    mAutogenerateMipmaps(false)
{
    LogAssert(mNumDimensions >= 1 && mNumDimensions <= 3, "Invalid number of dimensions.");
    mType = GT_TEXTURE;

    // Zero-out all the level information.
    for (uint32_t level = 0; level < MAX_MIPMAP_LEVELS; ++level)
    {
        mLDimension[level][0] = 0;
        mLDimension[level][1] = 0;
        mLDimension[level][2] = 0;
        mLNumBytes[level] = 0;
    }

    for (uint32_t item = 0; item < mNumItems; ++item)
    {
        for (uint32_t level = 0; level < MAX_MIPMAP_LEVELS; ++level)
        {
            mLOffset[item][level] = 0;
        }
    }

    // The base level is always used.
    mLDimension[0][0] = dim0;
    mLDimension[0][1] = dim1;
    mLDimension[0][2] = dim2;
    mLNumBytes[0] = dim0*dim1*dim2*mElementSize;

    if (mHasMipmaps)
    {
        uint32_t log0 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim0));
        uint32_t log1 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim1));
        uint32_t log2 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim2));
        mNumLevels = 1 + std::max(log0, std::max(log1, log2));
        for (uint32_t level = 1; level < mNumLevels; ++level)
        {
            if (dim0 > 1)
            {
                dim0 >>= 1;
            }

            if (dim1 > 1)
            {
                dim1 >>= 1;
            }

            if (dim2 > 1)
            {
                dim2 >>= 1;
            }

            mLNumBytes[level] = dim0 * dim1 * dim2 * mElementSize;
            mLDimension[level][0] = dim0;
            mLDimension[level][1] = dim1;
            mLDimension[level][2] = dim2;
        }

        uint32_t numBytes = 0;
        for (uint32_t item = 0; item < mNumItems; ++item)
        {
            for (uint32_t level = 0; level < mNumLevels; ++level)
            {
                mLOffset[item][level] = numBytes;
                numBytes += mLNumBytes[level];
            }
        }
    }
    else
    {
        for (uint32_t item = 1; item < mNumItems; ++item)
        {
            mLOffset[item][0] = item * mLNumBytes[0];
        }
    }
}

uint32_t Texture::GetIndex(uint32_t item, uint32_t level) const
{
    LogAssert(item < mNumItems && level < mNumLevels, "Invalid input.");
    return mNumLevels * item + level;
}

Texture::Subresource Texture::GetSubresource(uint32_t index) const
{
    LogAssert(index < GetNumSubresources(), "Invalid input.");
    Subresource sr{};
    sr.item = index / mNumLevels;
    sr.level = index % mNumLevels;
    sr.data = const_cast<char*>(GetDataFor(sr.item, sr.level));
    sr.rowPitch = mLDimension[sr.level][0] * mElementSize;
    sr.slicePitch = mLDimension[sr.level][1] * sr.rowPitch;
    return sr;
}

void Texture::AutogenerateMipmaps()
{
    if (mHasMipmaps)
    {
        // Mipmaps are generated internally on the GPU, so mUsage is
        // SHADER_OUTPUT.
        mAutogenerateMipmaps = true;
    }
}

uint32_t Texture::GetTotalElements(uint32_t numItems,
    uint32_t dim0, uint32_t dim1, uint32_t dim2, bool hasMipmaps)
{
    uint32_t numElementsPerItem = dim0 * dim1 * dim2;
    if (hasMipmaps)
    {
        uint32_t log0 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim0));
        uint32_t log1 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim1));
        uint32_t log2 = BitHacks::Log2OfPowerOfTwo(BitHacks::RoundDownToPowerOfTwo(dim2));
        uint32_t numLevels = 1 + std::max(log0, std::max(log1, log2));
        for (uint32_t level = 1; level < numLevels; ++level)
        {
            if (dim0 > 1)
            {
                dim0 >>= 1;
            }

            if (dim1 > 1)
            {
                dim1 >>= 1;
            }

            if (dim2 > 1)
            {
                dim2 >>= 1;
            }

            numElementsPerItem += dim0 * dim1 * dim2;
        }
    }

    uint32_t totalElements = numItems * numElementsPerItem;
    return totalElements;
}
