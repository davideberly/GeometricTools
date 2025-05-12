// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/Texture.h>
#include <Graphics/GL46/GL46Resource.h>
#include <array>

namespace gte
{
    class GL46Texture : public GL46Resource
    {
    public:
        // Abstract base class.
        virtual ~GL46Texture() = default;
    protected:
        GL46Texture(Texture const* texture, GLenum target, GLenum targetBinding);

    public:
        // Member access.
        inline Texture* GetTexture() const
        {
            return static_cast<Texture*>(mGTObject);
        }

        inline GLenum GetTarget() const
        {
            return mTarget;
        }

        inline GLenum GetTargetBinding() const
        {
            return mTargetBinding;
        }

        // Get the GL4 internal format for the specified Texture data format.
        inline static GLuint GetInternalFormat(uint32_t dataFormat)
        {
            return msGLTextureInternalFormat[dataFormat];
        }

    protected:
        GLenum mTarget;
        GLenum mTargetBinding;

        // Properties of overall texture.
        GLint mNumLevels;
        GLuint mInternalFormat;
        GLuint mExternalFormat;
        GLuint mExternalType;

        // Mapping from DFType to GL4 specific types
        static std::array<GLuint const, DF_NUM_FORMATS> msGLTextureInternalFormat;
        static std::array<GLuint const, DF_NUM_FORMATS> msGLTextureExternalFormat;
        static std::array<GLuint const, DF_NUM_CHANNEL_TYPES> msGLTextureExternalType;
    };
}

