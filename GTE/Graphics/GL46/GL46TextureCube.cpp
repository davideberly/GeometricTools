// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46TextureCube.h>
using namespace gte;

GL46TextureCube::~GL46TextureCube()
{
    glDeleteTextures(1, &mGLHandle);
}

GL46TextureCube::GL46TextureCube(TextureCube const* texture)
    :
    GL46TextureArray(texture, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, mNumLevels, mInternalFormat, width, height);

    Initialize();

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL46TextureCube::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_CUBE)
    {
        return std::make_shared<GL46TextureCube>(
            static_cast<TextureCube const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL46TextureCube::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL46TextureCube::LoadTextureLevel(uint32_t item, uint32_t level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto const width = texture->GetDimension(0);
        auto const height = texture->GetDimension(1);

        // Each face in the TextureCube has a unique GL target.
        GLenum targetFace = msCubeFaceTarget[item];

        glTexSubImage2D(targetFace, level, 0, 0, width, height,
            mExternalFormat, mExternalType, data);
    }
}

