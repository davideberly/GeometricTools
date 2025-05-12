// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureRT.h>
using namespace gte;

TextureRT::TextureRT(uint32_t format, uint32_t width, uint32_t height,
    bool hasMipmaps, bool createStorage)
    :
    Texture2(format, width, height, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE_RT;
}

