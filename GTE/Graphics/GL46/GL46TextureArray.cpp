// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2026.02.16

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46TextureArray.h>
using namespace gte;

GL46TextureArray::~GL46TextureArray()
{
    for (int32_t level = 0; level < mNumLevels; ++level)
    {
        glDeleteBuffers(1, &mLevelPixelUnpackBuffer[level]);
        glDeleteBuffers(1, &mLevelPixelPackBuffer[level]);
    }
}

GL46TextureArray::GL46TextureArray(TextureArray const* gtTexture, GLenum target, GLenum targetBinding)
    :
    GL46Texture(gtTexture, target, targetBinding)
{
    // Initially no staging buffers.
    std::fill(std::begin(mLevelPixelUnpackBuffer), std::end(mLevelPixelUnpackBuffer), 0);
    std::fill(std::begin(mLevelPixelPackBuffer), std::end(mLevelPixelPackBuffer), 0);
}

void GL46TextureArray::Initialize()
{
    // Save current binding for this texture target in order to restore it
    // when done because the gl texture object is needed to be bound to this
    // texture target for the operations to follow.
    GLint prevBinding;
    glGetIntegerv(mTargetBinding, &prevBinding);
    glBindTexture(mTarget, mGLHandle);

    // The default is 4-byte alignment.  This allows byte alignment when data
    // from user buffers into textures and vice versa.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Set the range of levels.
    glTexParameteri(mTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(mTarget, GL_TEXTURE_MAX_LEVEL, mNumLevels-1);

    // Initialize with data?
    auto texture = GetTexture();
    auto const numItems = texture->GetNumItems();
    if (texture->GetData())
    {
        if (CanAutoGenerateMipmaps())
        {
            // Initialize with the first mipmap level and then generate
            // the remaining mipmaps.
            for (uint32_t item = 0; item < numItems; ++item)
            {
                auto data = texture->GetDataFor(item, 0);
                if (data)
                {
                    LoadTextureLevel(item, 0, data);
                }
            }
            GenerateMipmaps();
        }
        else
        {
            // Initialize with each mipmap level.
            for (uint32_t item = 0; item < numItems; ++item)
            {
                for (int32_t level = 0; level < mNumLevels; ++level)
                {
                    auto data = texture->GetDataFor(item, level);
                    if (data)
                    {
                        LoadTextureLevel(item, level, data);
                    }
                }
            }
        }
    }

    glBindTexture(mTarget, prevBinding);
}

bool GL46TextureArray::Update()
{
    auto texture = GetTexture();
    auto const numItems = texture->GetNumItems();

    if (CanAutoGenerateMipmaps())
    {
        // Only update the level 0 texture and then generate the remaining
        // mipmaps from it.
        for (uint32_t item = 0; item < numItems; ++item)
        {
            if (!Update(item, 0))
            {
                return false;
            }
        }
        GenerateMipmaps();
    }
    else
    {
        // Automatic generation of mipmaps is not enabled, so all mipmap
        // levels must be copied to the GPU.
        auto const numLevels = texture->GetNumLevels();
        for (uint32_t item = 0; item < numItems; ++item)
        {
            for (uint32_t level = 0; level < numLevels; ++level)
            {
                if (!Update(item, level))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool GL46TextureArray::CopyCpuToGpu()
{
    auto texture = GetTexture();
    auto const numItems = texture->GetNumItems();

    if (CanAutoGenerateMipmaps())
    {
        // Only update the level 0 texture and then generate the remaining
        // mipmaps from it.
        for (uint32_t item = 0; item < numItems; ++item)
        {
            if (!CopyCpuToGpu(item, 0))
            {
                return false;
            }
        }
        GenerateMipmaps();
    }
    else
    {
        // Automatic generation of mipmaps is not enabled, so all mipmap
        // levels must be copied to the GPU.
        auto const numLevels = texture->GetNumLevels();
        for (uint32_t item = 0; item < numItems; ++item)
        {
            for (uint32_t level = 0; level < numLevels; ++level)
            {
                if (!CopyCpuToGpu(item, level))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool GL46TextureArray::CopyGpuToCpu()
{
    auto texture = GetTexture();
    auto const numItems = texture->GetNumItems();
    auto const numLevels = texture->GetNumLevels();
    for (uint32_t item = 0; item < numItems; ++item)
    {
        for (uint32_t level = 0; level < numLevels; ++level)
        {
            if (!CopyGpuToCpu(item, level))
            {
                return false;
            }
        }
    }

    return true;
}

bool GL46TextureArray::Update(uint32_t item, uint32_t level)
{
    auto texture = GetTexture();
    LogAssert(texture->GetUsage() == Resource::Usage::DYNAMIC_UPDATE,
        "Texture usage must be DYNAMIC_UPDATE.");
    return DoCopyCpuToGpu(item, level);
}

bool GL46TextureArray::CopyCpuToGpu(uint32_t item, uint32_t level)
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    return DoCopyCpuToGpu(item, level);
}

bool GL46TextureArray::CopyGpuToCpu(uint32_t item, uint32_t level)
{
    if (!PreparedForCopy(GL_READ_ONLY))
    {
        return false;
    }

    auto texture = GetTexture();

    // Make sure item is valid.
    auto const numItems = texture->GetNumItems();
    if (item >= numItems)
    {
        LogError("Item for texture is out of range");
    }

    // Make sure level is valid.
    auto const numLevels = texture->GetNumLevels();
    if (level >= numLevels)
    {
        LogError("Level for texture is out of range");
    }

    auto pixBuffer = mLevelPixelPackBuffer[level];
    if (0 == pixBuffer)
    {
        LogError("Staging buffer not defined for level " + std::to_string(level));
    }

    auto data = texture->GetDataFor(item, level);
    auto numBytes = texture->GetNumBytesFor(level);
    if ((nullptr == data) || (0 == numBytes))
    {
        LogError("No target data for texture level " + std::to_string(level));
    }

    auto const target = GetTarget();
    glBindTexture(target, mGLHandle);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffer);
    glGetTexImage(target, level, mExternalFormat, mExternalType, 0);
    glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, numBytes, data);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glBindTexture(target, 0);

    return true;
}

bool GL46TextureArray::GenerateMipmaps()
{
    if (CanAutoGenerateMipmaps())
    {
        // Save current binding for this texture target in order to restore it
        // when done because the gl texture object is needed to be bound to
        // this texture target for the operations to follow.
        GLint prevBinding;
        glGetIntegerv(mTargetBinding, &prevBinding);
        glBindTexture(mTarget, mGLHandle);

        // Generate the mipmaps.  All of this binding save and restore is not
        // necessary in OpenGL 4.6, where glGenerateTextureMipamap(mGLHandle)
        // can simply be used.
        glGenerateMipmap(mTarget);

        glBindTexture(mTarget, prevBinding);

        return true;
    }
    return false;
}

bool GL46TextureArray::DoCopyCpuToGpu(uint32_t item, uint32_t level)
{
    auto texture = GetTexture();

    // Cannot update automatically generated mipmaps in GPU.
    if (CanAutoGenerateMipmaps() && (level > 0))
    {
        LogError("Cannot update automatically generated mipmaps in GPU");
    }

    // Make sure item is valid.
    auto const numItems = texture->GetNumItems();
    if (item >= numItems)
    {
        LogError("Item for texture array is out of range");
    }

    // Make sure level is valid.
    auto const numLevels = texture->GetNumLevels();
    if (level >= numLevels)
    {
        LogError("Level for texture array is out of range");
    }

    auto data = texture->GetDataFor(item, level);
    auto numBytes = texture->GetNumBytesFor(level);
    if ((nullptr == data) || (0 == numBytes))
    {
        LogError("No source data for texture array level " + std::to_string(level));
    }

    auto const target = GetTarget();
    glBindTexture(target, mGLHandle);

    // Use staging buffer if present.
    auto pixBuffer = mLevelPixelUnpackBuffer[level];
    if (0 != pixBuffer)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffer);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, numBytes, data);
        LoadTextureLevel(item, level, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
    else
    {
        LoadTextureLevel(item, level, data);
    }

    glBindTexture(target, 0);

    return true;
}

void GL46TextureArray::CreateStaging()
{
    auto texture = GetTexture();
    auto const copyType = texture->GetCopy();

    auto const createPixelUnpackBuffers = 
        (copyType == Resource::Copy::CPU_TO_STAGING) ||
        (copyType == Resource::Copy::BIDIRECTIONAL);

    auto const createPixelPackBuffers =
        (copyType == Resource::Copy::STAGING_TO_CPU) ||
        (copyType == Resource::Copy::BIDIRECTIONAL);

    // TODO:  Determine frequency and nature of usage for this staging buffer
    // when created by calling glBufferData.
    GLenum usage = GL_DYNAMIC_DRAW;

    if (createPixelUnpackBuffers)
    {
        for (int32_t level = 0; level < mNumLevels; ++level)
        {
            auto& pixBuffer = mLevelPixelUnpackBuffer[level];
            if (0 == pixBuffer)
            {
                auto numBytes = texture->GetNumBytesFor(level);

                // Create pixel buffer for staging.
                glGenBuffers(1, &pixBuffer);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pixBuffer);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, numBytes, 0, usage);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            }
        }
    }

    if (createPixelPackBuffers)
    {
        for (int32_t level = 0; level < mNumLevels; ++level)
        {
            auto& pixBuffer = mLevelPixelPackBuffer[level];
            if (0 == pixBuffer)
            {
                auto numBytes = texture->GetNumBytesFor(level);

                // Create pixel buffer for staging.
                glGenBuffers(1, &pixBuffer);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, pixBuffer);
                glBufferData(GL_PIXEL_PACK_BUFFER, numBytes, 0, usage);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            }
        }
    }
}


GLenum const GL46TextureArray::msCubeFaceTarget[6] =
{
    GL_TEXTURE_CUBE_MAP_POSITIVE_X, // CubeFacePositiveX
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // CubeFaceNegativeX
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // CubeFacePositiveY
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // CubeFaceNegativeY
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // CubeFacePositiveZ
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // CubeFaceNegativeZ
};


