// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/GEDrawTarget.h>
#include <Graphics/GL46/GL46TextureDS.h>
#include <Graphics/GL46/GL46TextureRT.h>

namespace gte
{
    class GL46DrawTarget : public GEDrawTarget
    {
    public:
        // Construction and destruction.
        virtual ~GL46DrawTarget();
        GL46DrawTarget(DrawTarget const* target,
            std::vector<GL46TextureRT*>& rtTextures, GL46TextureDS* dsTexture);
        static std::shared_ptr<GEDrawTarget> Create(DrawTarget const* target,
            std::vector<GEObject*>& rtTextures, GEObject* dsTexture);

        // Member access.
        inline GL46TextureRT* GetRTTexture(uint32_t i) const
        {
            return mRTTextures[i];
        }

        inline GL46TextureDS* GetDSTexture() const
        {
            return mDSTexture;
        }

        // Used in the Renderer::Draw function.
        void Enable();
        void Disable();

    private:
        std::vector<GL46TextureRT*> mRTTextures;
        GL46TextureDS* mDSTexture;

        GLuint mFrameBuffer;

        // Temporary storage during enable/disable of targets.
        GLint mSaveViewportX;
        GLint mSaveViewportY;
        GLsizei mSaveViewportWidth;
        GLsizei mSaveViewportHeight;
        GLdouble mSaveViewportNear;
        GLdouble mSaveViewportFar;
    };
}

