// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/TextureDS.h>
#include <Graphics/GL46/GL46Texture2.h>

namespace gte
{
    class GL46TextureDS : public GL46Texture2
    {
    public:
        // Construction.
        virtual ~GL46TextureDS() = default;
        GL46TextureDS(TextureDS const* texture);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline TextureDS* GetTexture() const
        {
            return static_cast<TextureDS*>(mGTObject);
        }

        // Returns true of mipmaps need to be generated.
        virtual bool CanAutoGenerateMipmaps() const override;

    private:
    };
}

