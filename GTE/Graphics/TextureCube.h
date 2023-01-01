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
    class TextureCube : public TextureArray
    {
    public:
        // Construction.  Cube maps must be square; the 'length' parameter is
        // the shared value for width and height.
        TextureCube(uint32_t format, uint32_t length, bool hasMipmaps = false,
            bool createStorage = true);

        // The texture width and height are the same value.
        inline uint32_t GetLength() const
        {
            return TextureArray::GetDimension(0);
        }
    };
}
