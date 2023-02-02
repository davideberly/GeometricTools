// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureSingle.h>
#include <Graphics/GL45/GL45Texture.h>

namespace gte
{
    class GL45TextureSingle : public GL45Texture
    {
    public:
        // Abstract base class, a shim to distinguish between single textures
        // and texture arrays.
        virtual ~GL45TextureSingle();
    protected:
        // No public construction.  Derived classes use this constructor.
        GL45TextureSingle(TextureSingle const* gtTexture, GLenum target, GLenum targetBinding);

        // Only call from derived class constructor after texture storage has
        // been allocated.
        void Initialize();

    public:
        // Member access.
        inline TextureSingle* GetTexture() const
        {
            return static_cast<TextureSingle*>(mGTObject);
        }

        virtual bool Update() override;
        virtual bool CopyCpuToGpu() override;
        virtual bool CopyGpuToCpu() override;

        bool Update(uint32_t level);
        bool CopyCpuToGpu(uint32_t level);
        bool CopyGpuToCpu(uint32_t level);

        void CopyLevelGpuToGpu(GL45TextureSingle* texture, uint32_t level)
        {
            (void)texture;
            (void)level;
            LogError("Not yet implemented.");
        }

        // Returns true if mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const = 0;

        // Generates mipmaps from level 0 -- only if CanAutoGenerateMipmaps()
        // returns true.
        virtual bool GenerateMipmaps();

    protected:
        // Called by Update and CopyCpuToGpu.
        bool DoCopyCpuToGpu(uint32_t level);

        // Should be called in constructor when CopyType is any value but
        // COPY_NONE.
        void CreateStaging();

        // This is called to copy the data from the CPU buffer to the GPU
        // for the specified level.  If a pixel unpack buffer is being used
        // then data needs to be passed as 0 which is used as an offset.
        virtual void LoadTextureLevel(uint32_t level, void const* data) = 0;

    private:
        // Data associated with each mip level
        GLuint mLevelPixelUnpackBuffer[Texture::MAX_MIPMAP_LEVELS];
        GLuint mLevelPixelPackBuffer[Texture::MAX_MIPMAP_LEVELS];
    };
}
