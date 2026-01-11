// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46TextureDS.h>
using namespace gte;

GL46TextureDS::GL46TextureDS(TextureDS const* texture)
    :
    GL46Texture2(texture)
{
}

std::shared_ptr<GEObject> GL46TextureDS::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_DS)
    {
        return std::make_shared<GL46TextureDS>(
            static_cast<TextureDS const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL46TextureDS::CanAutoGenerateMipmaps() const
{
    return false;
}


