// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/TextureCubeArray.h>
#include <Graphics/GL45/GL45TextureArray.h>
#include <cstdint>

namespace gte
{
    class GL45TextureCubeArray : public GL45TextureArray
    {
    public:
        // Construction and destruction.
        virtual ~GL45TextureCubeArray();
        GL45TextureCubeArray(TextureCubeArray const* texture);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline TextureCubeArray* GetTexture() const
        {
            return static_cast<TextureCubeArray*>(mGTObject);
        }

        // Returns true if mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;

    protected:
        virtual void LoadTextureLevel(uint32_t item, uint32_t level, void const* data) override;
    };
}
