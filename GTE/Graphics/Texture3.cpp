// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Texture3.h>
using namespace gte;

Texture3::Texture3(uint32_t format, uint32_t width, uint32_t height,
    uint32_t thickness, bool hasMipmaps, bool createStorage)
    :
    TextureSingle(format, 3, width, height, thickness, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE3;
}
