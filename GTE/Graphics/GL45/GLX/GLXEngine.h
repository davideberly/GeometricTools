// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/GL45/GL45Engine.h>

// Forward declarations to avoid name conflicts caused by #include-ing
// X11/Xlib.h and GL/glx.h.
struct _XDisplay;
struct __GLXcontextRec;

namespace gte
{
    class GLXEngine : public GL45Engine
    {
    public:
        // Construction and destruction.  The first constructor is for windowed
        // graphics applications.  The second constructor is for windowless
        // compute-program applications.
        //
        // TODO: Currently, OpenGL 4.3 is required for compute shaders.  See
        // the comment in GteGL4Engine.h for 'MeetsRequirements()'.
        virtual ~GLXEngine();
        GLXEngine(_XDisplay* display, unsigned long window, __GLXcontextRec* context,
            int32_t xSize, int32_t ySize, bool useDepth24Stencil8, bool saveDriverInfo, int32_t requiredMajor = 4, int32_t requiredMinor = 3);
        GLXEngine(bool useDepth24Stencil8 = true, bool saveDriverInfo = false, int32_t requiredMajor = 4, int32_t requiredMinor = 3);

        // Member access.
        inline _XDisplay* GetDisplay() const
        {
            return mDisplay;
        }

        inline unsigned long GetWindow() const
        {
            return mWindow;
        }

        inline __GLXcontextRec* GetImmediate() const
        {
            return mImmediate;
        }

        // Allow the user to switch between OpenGL contexts when there are
        // multiple instances of GL4Engine in an application.
        virtual bool IsActive() const override;
        virtual void MakeActive() override;

        // Support for clearing the color, depth, and stencil back buffers.
        virtual void DisplayColorBuffer(uint32_t syncInterval) override;

    private:
        // Helpers for construction and destruction.
        virtual bool Initialize(int32_t requiredMajor, int32_t requiredMinor, bool useDepth24Stencil8, bool saveDriverInfo) override;
        void Terminate();

        _XDisplay* mDisplay;
        unsigned long mWindow;
        __GLXcontextRec* mImmediate;
        bool mIsComputeWindow;
    };
}
