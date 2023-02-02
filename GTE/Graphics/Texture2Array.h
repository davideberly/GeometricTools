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
    class Texture2Array : public TextureArray
    {
    public:
        // Construction.
        Texture2Array(uint32_t numItems, uint32_t format, uint32_t width,
            uint32_t height, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline uint32_t GetWidth() const
        {
            return TextureArray::GetDimension(0);
        }

        inline uint32_t GetHeight() const
        {
            return TextureArray::GetDimension(1);
        }
    };
}
