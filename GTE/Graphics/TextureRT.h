// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Texture2.h>
#include <cstdint>

namespace gte
{
    class TextureRT : public Texture2
    {
    public:
        // Construction for render targets.
        TextureRT(uint32_t format, uint32_t width, uint32_t height,
            bool hasMipmaps = false, bool createStorage = true);
    };
}
