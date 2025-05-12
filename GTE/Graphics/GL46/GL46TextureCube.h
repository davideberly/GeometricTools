// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/TextureCube.h>
#include <Graphics/GL46/GL46TextureArray.h>

namespace gte
{
    class GL46TextureCube : public GL46TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~GL46TextureCube();
        GL46TextureCube(TextureCube const* textureArray);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline TextureCube* GetTexture() const
        {
            return static_cast<TextureCube*>(mGTObject);
        }

        // Returns true if mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;

    protected:
        virtual void LoadTextureLevel(uint32_t item, uint32_t level, void const* data) override;
    };
}

