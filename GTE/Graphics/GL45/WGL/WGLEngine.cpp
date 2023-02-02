// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Graphics/GL45/WGL/WGLEngine.h>
using namespace gte;

extern "C"
{
    extern int32_t __stdcall wglSwapIntervalEXT(int32_t interval);
    extern int32_t __stdcall wglGetSwapIntervalEXT(void);
}
extern void InitializeWGL();

WGLEngine::~WGLEngine()
{
    Terminate();
}

WGLEngine::WGLEngine(HWND handle, bool useDepth24Stencil8, bool saveDriverInfo, int32_t requiredMajor, int32_t requiredMinor)
    :
    GL45Engine(),
    mHandle(handle),
    mDevice(nullptr),
    mImmediate(nullptr),
    mComputeWindowAtom(0)
{
    Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
}

WGLEngine::WGLEngine(bool useDepth24Stencil8, bool saveDriverInfo, int32_t requiredMajor, int32_t requiredMinor)
    :
    GL45Engine(),
    mHandle(nullptr),
    mDevice(nullptr),
    mImmediate(nullptr),
    mComputeWindowAtom(0)
{
    mComputeWindowClass = L"GL4ComputeWindowClass" +
        std::to_wstring(reinterpret_cast<uint64_t>(this));

    WNDCLASS wc{};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = mComputeWindowClass.c_str();
    wc.lpszMenuName = nullptr;
    mComputeWindowAtom = RegisterClass(&wc);

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect = { 0, 0, 15, 15 };
    if (AdjustWindowRect(&rect, style, FALSE) != FALSE)
    {
        int32_t xSize = static_cast<int32_t>(rect.right - rect.left + 1);
        int32_t ySize = static_cast<int32_t>(rect.bottom - rect.top + 1);
        mHandle = CreateWindow(mComputeWindowClass.c_str(), L"", style,
            0, 0, xSize, ySize, nullptr, nullptr, nullptr, nullptr);
        Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
    }
    else
    {
        UnregisterClass(mComputeWindowClass.c_str(), 0);
        mComputeWindowAtom = 0;
    }
}

bool WGLEngine::IsActive() const
{
    return mImmediate == wglGetCurrentContext();
}

void WGLEngine::MakeActive()
{
    if (mImmediate != wglGetCurrentContext())
    {
        wglMakeCurrent(mDevice, mImmediate);
    }
}

void WGLEngine::DisplayColorBuffer(uint32_t syncInterval)
{
    wglSwapIntervalEXT(syncInterval > 0 ? 1 : 0);
    SwapBuffers(mDevice);
}

bool WGLEngine::Initialize(int32_t requiredMajor, int32_t requiredMinor, bool useDepth24Stencil8, bool saveDriverInfo)
{
    if (!mHandle)
    {
        LogError("Invalid window handle.");
    }

    mDevice = GetDC(mHandle);
    if (!mDevice)
    {
        LogError("Invalid device context.");
    }

    RECT rect;
    BOOL result = GetClientRect(mHandle, &rect); (void)result;
    mXSize = static_cast<uint32_t>(rect.right - rect.left);
    mYSize = static_cast<uint32_t>(rect.bottom - rect.top);

    // Select the format for the drawing surface.
    PIXELFORMATDESCRIPTOR pfd;
    std::memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_GENERIC_ACCELERATED |
        PFD_DOUBLEBUFFER;

    // Create an R8G8B8A8 buffer.
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    // Create a depth-stencil buffer.
    if (useDepth24Stencil8)
    {
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
    }
    else
    {
        pfd.cDepthBits = 32;
        pfd.cStencilBits = 0;
    }

    // Set the pixel format for the rendering context.
    int32_t pixelFormat = ChoosePixelFormat(mDevice, &pfd);
    if (pixelFormat == 0)
    {
        LogError("ChoosePixelFormat failed.");
    }

    if (!SetPixelFormat(mDevice, pixelFormat, &pfd))
    {
        LogError("SetPixelFormat failed.");
    }

    // Create an OpenGL context.
    mImmediate = wglCreateContext(mDevice);
    if (!mImmediate)
    {
        LogError("wglCreateContext failed.");
    }

    // Activate the context.
    if (!wglMakeCurrent(mDevice, mImmediate))
    {
        LogError("wglMakeCurrent failed.");
    }

    // Get the function pointers for WGL.
    InitializeWGL();

    // Get the function pointers for OpenGL; initialize the viewport,
    // default global state, and default font.
    return GL45Engine::Initialize(requiredMajor, requiredMinor, useDepth24Stencil8, saveDriverInfo);
}

void WGLEngine::Terminate()
{
    GL45Engine::Terminate();

    if (mHandle)
    {
        if (mDevice && mImmediate)
        {
            wglMakeCurrent(mDevice, nullptr);
            wglDeleteContext(mImmediate);
            ReleaseDC(mHandle, mDevice);
        }

        if (mComputeWindowAtom)
        {
            DestroyWindow(mHandle);
            UnregisterClass(mComputeWindowClass.c_str(), 0);
        }
    }
}
