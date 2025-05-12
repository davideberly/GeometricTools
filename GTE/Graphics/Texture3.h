// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/TextureSingle.h>
#include <cstdint>

namespace gte
{
    class Texture3 : public TextureSingle
    {
    public:
        // Construction.
        Texture3(uint32_t format, uint32_t width, uint32_t height,
            uint32_t thickness, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline uint32_t GetWidth() const
        {
            return TextureSingle::GetDimension(0);
        }

        inline uint32_t GetHeight() const
        {
            return TextureSingle::GetDimension(1);
        }

        inline uint32_t GetThickness() const
        {
            return TextureSingle::GetDimension(2);
        }
    };
}

