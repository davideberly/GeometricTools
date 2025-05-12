// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46TextureCubeArray.h>
using namespace gte;

GL46TextureCubeArray::~GL46TextureCubeArray()
{
    glDeleteTextures(1, &mGLHandle);
}

GL46TextureCubeArray::GL46TextureCubeArray(TextureCubeArray const* texture)
    :
    GL46TextureArray(texture, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BINDING_CUBE_MAP_ARRAY)
{
    // Create a texture structure.
    glGenTextures(1, &mGLHandle);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
    auto const numItems = texture->GetNumItems();
    auto const numCubes = texture->GetNumCubes();
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, mNumLevels, mInternalFormat, width, height, numItems);

    // The default is 4-byte alignment.  This allows byte alignment when data
    // from user buffers into textures and vice versa.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Set the range of levels.
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAX_LEVEL, mNumLevels-1);

    // Initialize with data?
    if (texture->GetData())
    {
        if (CanAutoGenerateMipmaps())
        {
            // Initialize with the first mipmap level and then generate
            // the remaining mipmaps.
            for (uint32_t cube = 0; cube < numCubes; ++cube)
            {
                for (uint32_t face = 0; face < texture->cubeFaceCount; ++face)
                {
                    auto data = texture->GetDataFor(cube, face, 0);
                    if (data)
                    {
                        auto item = texture->GetItemIndexFor(cube, face);
                        LoadTextureLevel(item, 0, data);
                    }
                }
            }
            GenerateMipmaps();
        }
        else
        {
            // Initialize with each mipmap level.
            for (uint32_t cube = 0; cube < numCubes; ++cube)
            {
                for (uint32_t face = 0; face < texture->cubeFaceCount; ++face)
                {
                    for (int32_t level = 0; level < mNumLevels; ++level)
                    {
                        auto data = texture->GetDataFor(cube, face, level);
                        if (data)
                        {
                            auto item = texture->GetItemIndexFor(cube, face);
                            LoadTextureLevel(item, level, data);
                        }
                    }
                }
            }
        }
    }

    // Cannot leave this texture bound.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

std::shared_ptr<GEObject> GL46TextureCubeArray::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_CUBE_ARRAY)
    {
        return std::make_shared<GL46TextureCubeArray>(
            static_cast<TextureCubeArray const*>(object));
    }

    LogError("Invalid object type.");
}

bool GL46TextureCubeArray::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL46TextureCubeArray::LoadTextureLevel(uint32_t item, uint32_t level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto const width = texture->GetDimensionFor(level, 0);
        auto const height = texture->GetDimensionFor(level, 1);

        // Determine cube and face indexes from the item index.
        auto const cube = texture->GetCubeIndexFor(item);
        auto const face = texture->GetFaceIndexFor(item);

        // Each face in the TextureCubeArray has a unique GL target.
        GLenum targetFace = msCubeFaceTarget[face];

        // For TextureCubeArray, use the 3D calls where the cube index is the
        // third dimension.  Only updating one cube-face for the specified
        // level.
        glTexSubImage3D(targetFace, level, 0, 0, cube, width, height, 1,
            mExternalFormat, mExternalType, data);
    }
}

