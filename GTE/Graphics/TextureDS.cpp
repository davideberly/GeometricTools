// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/TextureDS.h>
using namespace gte;

TextureDS::TextureDS(uint32_t format, uint32_t width, uint32_t height,
    bool createStorage)
    :
    Texture2(DataFormat::IsDepth(format) ? format : DF_D24_UNORM_S8_UINT,
        width, height, false, createStorage),
    mShaderInput(false)
{
    mType = GT_TEXTURE_DS;
}
