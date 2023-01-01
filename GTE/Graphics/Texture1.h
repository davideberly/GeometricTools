// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureSingle.h>
#include <cstdint>

namespace gte
{
    class Texture1 : public TextureSingle
    {
    public:
        // Construction.
        Texture1(uint32_t format, uint32_t length, bool hasMipmaps = false, bool createStorage = true);

        // Texture dimensions.
        inline uint32_t GetLength() const
        {
            return TextureSingle::GetDimension(0);
        }
    };
}
