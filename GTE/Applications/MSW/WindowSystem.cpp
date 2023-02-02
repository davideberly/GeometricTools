// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.02

#include <Applications/GTApplicationsPCH.h>
#include <Applications/MSW/Window.h>
using namespace gte;

// The singleton used to create and destroy windows for applications.
namespace gte
{
    WindowSystem TheWindowSystem;
}

WindowSystem::~WindowSystem()
{
    if (mHandleMap.empty() && mAtom)
    {
        UnregisterClass(mWindowClassName, nullptr);
    }
}

WindowSystem::WindowSystem()
    :
    mWindowClassName(L"GTEngineWindow"),
    mAtom(0)
{
    WNDCLASS wc{};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProcedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = mWindowClassName;
    wc.lpszMenuName = nullptr;
    mAtom = RegisterClass(&wc);
}

bool WindowSystem::GetWindowRectangle(int32_t xClientSize, int32_t yClientSize,
    DWORD style, RECT& windowRectangle)
{
    windowRectangle.left = 0;
    windowRectangle.top = 0;
    windowRectangle.right = static_cast<LONG>(xClientSize) - 1;
    windowRectangle.bottom = static_cast<LONG>(yClientSize) - 1;
    return AdjustWindowRect(&windowRectangle, style, FALSE) != FALSE;
}

void WindowSystem::CreateFrom(Window::Parameters& parameters)
{
    DWORD style;
    if (parameters.allowResize)
    {
        style = WS_OVERLAPPEDWINDOW;
    }
    else
    {
        // This removes WS_THICKFRAME and WS_MAXIMIZEBOX from
        // WS_OVERLAPPEDWINDOW, both of which allow resizing of windows.
        style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    }

    if (parameters.hscrollBar)
    {
        style |= WS_HSCROLL;
    }

    if (parameters.vscrollBar)
    {
        style |= WS_VSCROLL;
    }

    RECT rectangle;
    bool adjusted = GetWindowRectangle(parameters.xSize, parameters.ySize,
        style, rectangle);
    if (adjusted)
    {
        int32_t adjustedXSize = (int32_t)rectangle.right - (int32_t)rectangle.left + 1;
        int32_t adjustedYSize = (int32_t)rectangle.bottom - (int32_t)rectangle.top + 1;
        parameters.handle = CreateWindow(mWindowClassName,
            parameters.title.c_str(), style, parameters.xOrigin,
            parameters.yOrigin, adjustedXSize, adjustedYSize, parameters.parent,
            nullptr, nullptr, nullptr);

        // AdjustWindowRect decides that scroll bars cover client space,
        // so the adjustment reduces the requested client rectangle size.
        // We have to detect this and recreate a window of the correct size.
        if (parameters.hscrollBar || parameters.vscrollBar)
        {
            GetClientRect(parameters.handle, &rectangle);
            int32_t clientXSize = (int32_t)rectangle.right - (int32_t)rectangle.left;
            int32_t clientYSize = (int32_t)rectangle.bottom - (int32_t)rectangle.top;
            if (clientXSize != parameters.xSize
                || clientYSize != parameters.ySize)
            {
                DestroyWindow(parameters.handle);
                adjustedXSize += parameters.xSize - clientXSize;
                adjustedYSize += parameters.ySize - clientYSize;
                parameters.handle = CreateWindow(mWindowClassName,
                    parameters.title.c_str(), style, parameters.xOrigin,
                    parameters.yOrigin, adjustedXSize, adjustedYSize,
                    parameters.parent, nullptr, nullptr, nullptr);
                GetClientRect(parameters.handle, &rectangle);
            }
        }

        CreateEngineAndProgramFactory(parameters);
    }
    else
    {
        LogError("AdjustWindowRect failed.");
    }
}

#if defined(GTE_USE_DIRECTX)

#include <Graphics/DX11/DXGIAdapter.h>

void WindowSystem::CreateEngineAndProgramFactory(Window::Parameters& parameters)
{
    // The adapterManager must be declared outside the if-then-else
    // statement so that it persists long enough to create the 'engine'
    // object.
    DXGIAdapter adapterManager;
    IDXGIAdapter* adapter = nullptr;
    if ((parameters.deviceCreationFlags & D3D11_CREATE_DEVICE_DEBUG) == 0)
    {
        // The GPU adapter is selected using the following algorithm. If a
        // discrete adapter is available (NVIDIA, AMD or other manufacturer),
        // it is selected. If a discrete adapter is not available, Intel
        // Integrated Graphics is chosen. Although these days Intel Core
        // architecture is the norm, in the event Intel Integrated Graphics is
        // not found, the fallback is to Microsoft WARP which is a software
        // implementation for DirectX 11 that is multithreaded and has decent
        // performance.
        adapterManager = DXGIAdapter::GetMostPowerful();
        adapter = adapterManager.GetAdapter();
    }
    else
    {
        // If parameters.deviceCreationFlags is 0 (no flags specified), the
        // first adapter in the adapter enumeration is selected. This is
        // invariable the adapter to which the display monitors are attached.
        //
        // If the debug layer is selected using D3D11_CREATE_DEVICE_DEBUG,
        // choosing a non-null adapter does not work. It will cause the
        // D3D11CreateDevice funtion to throw an exception and not return
        // an HRESULT code.
        adapter = nullptr;
    }

    auto engine = std::make_shared<DX11Engine>(adapter,
        parameters.handle, parameters.xSize, parameters.ySize,
        parameters.useDepth24Stencil8, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, parameters.deviceCreationFlags);

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<HLSLProgramFactory>();
        parameters.created = true;
    }
    else
    {
        LogError("Cannot create graphics engine.");
    }
}
#endif

#if defined(GTE_USE_OPENGL)
void WindowSystem::CreateEngineAndProgramFactory(Window::Parameters& parameters)
{
    bool saveDriverInfo = ((parameters.deviceCreationFlags & 0x00000001) != 0);
    auto engine = std::make_shared<WGLEngine>(parameters.handle,
        parameters.useDepth24Stencil8, saveDriverInfo);
    if (!engine->MeetsRequirements())
    {
        LogError("OpenGL 4.5 or later is required.");
    }

    if (engine->GetDevice())
    {
        parameters.engine = engine;
        parameters.factory = std::make_shared<GLSLProgramFactory>();
        parameters.created = true;
        engine->DisplayColorBuffer(0);
    }
    else
    {
        LogError("Cannot create graphics engine.");
    }
}
#endif

void WindowSystem::Extract(LPARAM lParam, int32_t& x, int32_t& y)
{
    x = static_cast<int32_t>(static_cast<int16_t>(lParam & 0xFFFF));
    y = static_cast<int32_t>(static_cast<int16_t>((lParam & 0xFFFF0000) >> 16));
}

void WindowSystem::Extract(WPARAM wParam, int32_t& x, int32_t& y)
{
    x = static_cast<int32_t>(static_cast<int16_t>(wParam & 0xFFFF));
    y = static_cast<int32_t>(static_cast<int16_t>((wParam & 0xFFFF0000) >> 16));
}

LRESULT CALLBACK WindowSystem::WindowProcedure(HWND handle, UINT message,
    WPARAM wParam, LPARAM lParam)
{
    auto iter = TheWindowSystem.mHandleMap.find(handle);
    if (iter == TheWindowSystem.mHandleMap.end())
    {
        return DefWindowProc(handle, message, wParam, lParam);
    }

    Window& window = *iter->second;

    LRESULT lResult = 0;
    if (window.OnWindowsMessage(handle, message, wParam, lParam, lResult))
    {
        // The 'window' does not want the message interpreted by the switch
        // statement below.
        return lResult;
    }

    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(handle, &ps);
        window.OnDisplay();
        EndPaint(handle, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
    {
        // This tells Windows not to erase the background (and that the
        // application is doing so).
        return 1;
    }
    case WM_MOVE:
    {
        // Get the origin of the moved window.  The y-value for window
        // moves is left-handed.
        int32_t x, y;
        Extract(lParam, x, y);
        window.OnMove(x, y);
        return 0;
    }
    case WM_SIZE:
    {
        // Get the new size of the window.
        int32_t xSize, ySize;
        Extract(lParam, xSize, ySize);

        if (wParam == SIZE_MINIMIZED)
        {
            // assert:  xSize == 0 and ySize == 0
            window.OnMinimize();
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            window.OnMaximize();
            window.OnResize(xSize, ySize);
        }
        else if (wParam == SIZE_RESTORED)
        {

            window.OnResize(xSize, ySize);
        }
        return 0;
    }
    case WM_CHAR:
    {
        // Get thet translated key code.
        uint8_t key = static_cast<uint8_t>(static_cast<char>(wParam));

        // Get the cursor position in client coordinates.
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        int32_t x = static_cast<int32_t>(point.x);
        int32_t y = static_cast<int32_t>(point.y);

        window.OnCharPress(key, x, y);
        return 0;
    }
    case WM_KEYDOWN:
    {
        // Get the virtual key code.
        int32_t key = static_cast<int32_t>(wParam);

        // Get the cursor position in client coordinates.
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        int32_t x = static_cast<int32_t>(point.x);
        int32_t y = static_cast<int32_t>(point.y);

        window.OnKeyDown(key, x, y);
        return 0;
    }
    case WM_KEYUP:
    {
        // Get the virtual key code.
        int32_t key = static_cast<int32_t>(wParam);

        // Get the cursor position in client coordinates.
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        int32_t x = static_cast<int32_t>(point.x);
        int32_t y = static_cast<int32_t>(point.y);

        window.OnKeyUp(key, x, y);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_LEFT, WindowApplication::MOUSE_DOWN,
            x, y, modifiers);
        return 0;
    }
    case WM_LBUTTONUP:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_LEFT, WindowApplication::MOUSE_UP,
            x, y, modifiers);
        return 0;
    }
    case WM_MBUTTONDOWN:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_MIDDLE, WindowApplication::MOUSE_DOWN,
            x, y, modifiers);
        return 0;
    }
    case WM_MBUTTONUP:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_MIDDLE, WindowApplication::MOUSE_UP,
            x, y, modifiers);
        return 0;
    }
    case WM_RBUTTONDOWN:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_RIGHT, WindowApplication::MOUSE_DOWN,
            x, y, modifiers);
        return 0;
    }
    case WM_RBUTTONUP:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        window.OnMouseClick(WindowApplication::MOUSE_RIGHT, WindowApplication::MOUSE_UP,
            x, y, modifiers);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        // Get the modifier flags.
        uint32_t modifiers = static_cast<uint32_t>(wParam);

        // Get the cursor position in client coordinates.
        int32_t x, y;
        Extract(lParam, x, y);

        int32_t button;
        if (wParam & MK_LBUTTON)
        {
            button = WindowApplication::MOUSE_LEFT;
        }
        else if (wParam & MK_MBUTTON)
        {
            button = WindowApplication::MOUSE_MIDDLE;
        }
        else if (wParam & MK_RBUTTON)
        {
            button = WindowApplication::MOUSE_RIGHT;
        }
        else
        {
            button = WindowApplication::MOUSE_NONE;
        }

        window.OnMouseMotion(button, x, y, modifiers);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        // Get the modifier flags and the amount the wheel rotated.
        int32_t modifiers, delta;
        Extract(wParam, modifiers, delta);

        // Get the cursor position in client coordinates.
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(handle, &point);
        int32_t x = static_cast<int32_t>(point.x);
        int32_t y = static_cast<int32_t>(point.y);

        window.OnMouseWheel(delta, x, y, (uint32_t)modifiers);
        return 0;
    }
    case WM_HSCROLL:  // 0x0114
    case WM_VSCROLL:  // 0x0115
    {
        int32_t bar = message - WM_HSCROLL;  // 0 or 1
        switch (LOWORD(wParam))
        {
        case SB_LINELEFT:
            window.OnScrollDecrementLoRes(bar);
            break;
        case SB_LINERIGHT:
            window.OnScrollIncrementLoRes(bar);
            break;
        case SB_PAGELEFT:
            window.OnScrollDecrementHiRes(bar);
            break;
        case SB_PAGERIGHT:
            window.OnScrollIncrementHiRes(bar);
            break;
        case SB_THUMBPOSITION:
            window.OnScrollEndTracking(bar);
            break;
        case SB_THUMBTRACK:
            window.OnScrollTracking(bar);
            break;
        default:
            // Not handled:  SB_LEFT, SB_RIGHT, SB_ENDSCROLL
            break;
        }
        return 0;
    }
    case WM_CLOSE:
    {
        window.OnClose();
        return 0;
    }
    case WM_COPYDATA:
    {
        window.OnCopyData((HWND)wParam, reinterpret_cast<PCOPYDATASTRUCT>(lParam));
        return 1;
    }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}
