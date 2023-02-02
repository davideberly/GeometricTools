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
    class Texture1Array : public TextureArray
    {
    public:
        // Construction.
        Texture1Array(uint32_t numItems, uint32_t format, uint32_t length,
            bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline uint32_t GetLength() const
        {
            return TextureArray::GetDimension(0);
        }
    };
}
