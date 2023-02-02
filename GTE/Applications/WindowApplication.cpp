// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.02

#include <Applications/GTApplicationsPCH.h>
#include <Applications/WindowApplication.h>
using namespace gte;

WindowApplication::Parameters::Parameters()
    :
    title(L""),
    xOrigin(0),
    yOrigin(0),
    xSize(0),
    ySize(0),
    allowResize(false),
    useDepth24Stencil8(true),
    created(false)
{
}

WindowApplication::Parameters::Parameters(std::wstring const& inTitle,
    int32_t inXOrigin, int32_t inYOrigin, int32_t inXSize, int32_t inYSize)
    :
    title(inTitle),
    xOrigin(inXOrigin),
    yOrigin(inYOrigin),
    xSize(inXSize),
    ySize(inYSize),
    allowResize(false),
    useDepth24Stencil8(true),
    created(false)
{
}

WindowApplication::WindowApplication(Parameters& parameters)
    :
    Application(parameters),
    mTitle(parameters.title),
    mXOrigin(parameters.xOrigin),
    mYOrigin(parameters.yOrigin),
    mXSize(parameters.xSize),
    mYSize(parameters.ySize),
    mAllowResize(parameters.allowResize),
    mIsMinimized(false),
    mIsMaximized(false)
{
}

void WindowApplication::OnMove(int32_t x, int32_t y)
{
    mXOrigin = x;
    mYOrigin = y;
}

bool WindowApplication::OnResize(int32_t xSize, int32_t ySize)
{
    mIsMinimized = false;
    mIsMaximized = false;

    if (xSize != mXSize || ySize != mYSize)
    {
        mXSize = xSize;
        mYSize = ySize;

        if (mBaseEngine)
        {
            mBaseEngine->Resize(xSize, ySize);
        }
        return true;
    }

    return false;
}

void WindowApplication::OnMinimize()
{
    mIsMinimized = true;
    mIsMaximized = false;
}

void WindowApplication::OnMaximize()
{
    mIsMinimized = false;
    mIsMaximized = true;
}

void WindowApplication::OnDisplay()
{
    // Stub for derived classes.
}

void WindowApplication::OnIdle()
{
    // Stub for derived classes.
}

bool WindowApplication::OnCharPress(uint8_t key, int32_t, int32_t)
{
    if (key == KEY_ESCAPE)
    {
        // Quit the application when the 'escape' key is pressed.
        OnClose();
        return true;
    }

    if (key == ' ')
    {
        mTimer.Reset();
        return true;
    }

    return false;
}

bool WindowApplication::OnKeyDown(int32_t, int32_t, int32_t)
{
    // Stub for derived classes.
    return false;
}

bool WindowApplication::OnKeyUp(int32_t, int32_t, int32_t)
{
    // Stub for derived classes.
    return false;
}

bool WindowApplication::OnMouseClick(int32_t, int32_t, int32_t, int32_t, uint32_t)
{
    // stub for derived classes
    return false;
}

bool WindowApplication::OnMouseMotion(int32_t, int32_t, int32_t, uint32_t)
{
    // stub for derived classes
    return false;
}

bool WindowApplication::OnMouseWheel(int32_t, int32_t, int32_t, uint32_t)
{
    // Stub for derived classes.
    return false;
}

void WindowApplication::SetMousePosition(int32_t, int32_t)
{
    // Stub for derived classes.
}

void WindowApplication::GetMousePosition(int32_t&, int32_t&) const
{
    // Stub for derived classes.
}

#if defined(GTE_USE_MSWINDOWS)
void WindowApplication::OnCopyData(HWND, PCOPYDATASTRUCT)
{
    // Stub for derived classes.
}
#endif

void WindowApplication::OnClose()
{
    // Stub for derived classes.
}

#if defined(GTE_USE_MSWINDOWS)
bool WindowApplication::OnWindowsMessage(HWND, UINT,
    WPARAM, LPARAM, LRESULT& result)
{
    result = 0;
    return false;
}
#endif
