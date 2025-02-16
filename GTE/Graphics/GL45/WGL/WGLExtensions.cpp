// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.11.14

#include <Graphics/GL45/GL45.h>
#include <Graphics/GL45/GL/wglext.h>
#include <cassert>
#include <cstdint>

void* GetOpenGLFunctionPointer(char const* name)
{
    return reinterpret_cast<void*>(wglGetProcAddress(name));
}

static PFNWGLSWAPINTERVALEXTPROC swglSwapIntervalEXT = nullptr;
static PFNWGLGETSWAPINTERVALEXTPROC swglGetSwapIntervalEXT = nullptr;

BOOL __stdcall wglSwapIntervalEXT(int interval)
{
    if (swglSwapIntervalEXT)
    {
        return swglSwapIntervalEXT(interval);
    }
    else
    {
        // The swap intervals extension is required.
        assert(false);
        return FALSE;
    }
}

int __stdcall wglGetSwapIntervalEXT(void)
{
    if (swglGetSwapIntervalEXT)
    {
        return swglGetSwapIntervalEXT();
    }
    else
    {
        // The swap intervals extension is required.
        assert(false);
        return 0;
    }
}

void InitializeWGL()
{
    swglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
        GetOpenGLFunctionPointer("wglSwapIntervalEXT"));

    swglGetSwapIntervalEXT = reinterpret_cast<PFNWGLGETSWAPINTERVALEXTPROC>(
        GetOpenGLFunctionPointer("wglGetSwapIntervalEXT"));
}
