// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureCube.h>
#include <Graphics/GL45/GL45TextureArray.h>

namespace gte
{
    class GL45TextureCube : public GL45TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~GL45TextureCube();
        GL45TextureCube(TextureCube const* textureArray);
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
