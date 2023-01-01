// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureArray.h>
#include <cstdint>

namespace gte
{
    class TextureCubeArray : public TextureArray
    {
    public:
        // Construction.  Cube maps must be square; the 'length' parameter is
        // the shared value for width and height of a face.  The 'numCubes' is
        // the number of 6-tuples of cube maps.
        TextureCubeArray(uint32_t numCubes, uint32_t format, uint32_t length,
            bool hasMipmaps = false, bool createStorage = true);

        // Member access.
        inline uint32_t GetNumCubes() const
        {
            return mNumCubes;
        }

        // The texture width and height are the same value.
        inline uint32_t GetLength() const
        {
            return TextureArray::GetDimension(0);
        }

        // Faces for all the of the cubes are stored contiguously in one large
        // array so GetNumItems() will return a number that is the same as
        // 6*GetNumCubes().  These methods allow mapping between the array
        // itemIndex and the corresponding (cubeIndex, faceIndex) pair.
        inline uint32_t GetItemIndexFor(uint32_t cube, uint32_t face) const
        {
            return cube * cubeFaceCount + face;
        }

        inline uint32_t GetCubeIndexFor(uint32_t item) const
        {
            return item / cubeFaceCount;
        }

        inline uint32_t GetFaceIndexFor(uint32_t item) const
        {
            return item % cubeFaceCount;
        }

        // Mipmap information.
        inline uint32_t GetOffsetFor(uint32_t cube, uint32_t face, uint32_t level) const
        {
            return TextureArray::GetOffsetFor(GetItemIndexFor(cube, face), level);
        }

        inline char const* GetDataFor(uint32_t cube, uint32_t face, uint32_t level) const
        {
            return TextureArray::GetDataFor(GetItemIndexFor(cube, face), level);
        }

        inline char* GetDataFor(uint32_t cube, uint32_t face, uint32_t level)
        {
            return TextureArray::GetDataFor(GetItemIndexFor(cube, face), level);
        }

        template <typename T>
        inline T const* GetFor(uint32_t cube, uint32_t face, uint32_t level) const
        {
            return TextureArray::GetFor<T>(GetItemIndexFor(cube, face), level);
        }

        template <typename T>
        inline T* GetFor(uint32_t cube, uint32_t face, uint32_t level)
        {
            return TextureArray::GetFor<T>(GetItemIndexFor(cube, face), level);
        }

        // Subresource indexing:  index = numLevels*item + level
        // where item = cube*6 + face
        inline uint32_t GetIndex(uint32_t cube, uint32_t face, uint32_t level) const
        {
            return mNumLevels * (cubeFaceCount * cube + face) + level;
        }

    private:
        uint32_t mNumCubes;
    };
}
