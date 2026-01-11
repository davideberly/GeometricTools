// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/DepthStencilState.h>
#include <Graphics/GL46/GL46DrawingState.h>

namespace gte
{
    class GL46DepthStencilState : public GL46DrawingState
    {
    public:
        // Construction.
        virtual ~GL46DepthStencilState() = default;
        GL46DepthStencilState(DepthStencilState const* depthStencilState);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline DepthStencilState* GetDepthStencilState()
        {
            return static_cast<DepthStencilState*>(mGTObject);
        }

        // Enable the depth-stencil state.
        void Enable();

    private:
        struct Face
        {
            GLenum onFail;
            GLenum onZFail;
            GLenum onZPass;
            GLenum comparison;
        };

        GLboolean mDepthEnable;
        GLboolean mWriteMask;
        GLenum mComparison;
        GLboolean mStencilEnable;
        GLuint mStencilReadMask;
        GLuint mStencilWriteMask;
        Face mFrontFace;
        Face mBackFace;
        GLuint mReference;

        // Conversions from GTEngine values to GL4 values.
        static GLboolean const msWriteMask[];
        static GLenum const msComparison[];
        static GLenum const msOperation[];
    };
}


