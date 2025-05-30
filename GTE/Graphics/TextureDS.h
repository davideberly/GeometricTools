// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Texture2.h>
#include <cstdint>

namespace gte
{
    class TextureDS : public Texture2
    {
    public:
        // Construction for depth-stencil textures.
        TextureDS(uint32_t format, uint32_t width, uint32_t height, bool createStorage = true);

        inline void MakeShaderInput()
        {
            mShaderInput = true;
        }

        inline bool IsShaderInput() const
        {
            return mShaderInput;
        }

    private:
        bool mShaderInput;
    };
}

