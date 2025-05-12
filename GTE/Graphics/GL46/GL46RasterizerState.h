// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/RasterizerState.h>
#include <Graphics/GL46/GL46DrawingState.h>

namespace gte
{
    class GL46RasterizerState : public GL46DrawingState
    {
    public:
        // Construction.
        virtual ~GL46RasterizerState() = default;
        GL46RasterizerState(RasterizerState const* rasterizerState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline RasterizerState* GetRasterizerState()
        {
            return static_cast<RasterizerState*>(mGTObject);
        }

        // Enable the rasterizer state.
        void Enable();

    private:
        GLenum mFillMode;
        GLenum mCullFace;
        GLenum mFrontFace;
        float mDepthScale;
        float mDepthBias;
        GLboolean mEnableScissor;

        // TODO: D3D11_RASTERIZER_DESC has the following.  We need to determine
        // how to handle these in OpenGL.
        //   DepthBiasClamp
        //   DepthClipEnable
        //   MultisampleEnable
        //   AntialiasedLineEnable

        // Conversions from GTEngine values to GL46 values.
        static GLenum const msFillMode[];
        static GLenum const msCullFace[];
    };
}

