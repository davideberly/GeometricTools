// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GL46.h>
#include <EGL/egl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

bool gUseEGLGetProcAddress = false;

void* GetOpenGLFunctionPointer(char const* name)
{
    if (gUseEGLGetProcAddress)
    {
        return (void*)(*eglGetProcAddress)(name);
    }
    else
    {
        return (void*)(*glXGetProcAddress)((GLubyte const*)name);
    }
}

struct SwapIntervalLoader
{
    SwapIntervalLoader()
        :
        sglXSwapInterval(nullptr)
    {
        sglXSwapInterval = (PFNGLXSWAPINTERVALEXTPROC)((void*)(*glXGetProcAddress)((GLubyte const*)name));
    }
    
    ~SwapIntervalLoader()
    {
        sglXSwapInterval = nullptr;
    }

    char const* name = "glXSwapIntervalEXT";
    PFNGLXSWAPINTERVALEXTPROC sglXSwapInterval;
};

static SwapIntervalLoader gLoader{};

void glXSwapInterval(Display* display, unsigned long window, int syncInterval)
{
    if (gLoader.sglXSwapInterval)
    {
        gLoader.sglXSwapInterval(display, window, syncInterval);
        return;
    }
}



