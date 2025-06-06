// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureSingle.h>
using namespace gte;

TextureSingle::TextureSingle(uint32_t format, uint32_t numDimensions,
    uint32_t dim0, uint32_t dim1, uint32_t dim2, bool hasMipmaps,
    bool createStorage)
    :
    Texture(1, format, numDimensions, dim0, dim1, dim2, hasMipmaps, createStorage)
{
    mType = GT_TEXTURE_SINGLE;
}

