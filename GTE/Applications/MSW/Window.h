// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.02

#pragma once

// Turn off the min/max macros to avoid conflict with std::min/std::max
#if !defined(NOMINMAX)
#define NOMINMAX
#define GTE_NOMINMAX_WAS_SET
#endif
#include <Windows.h>
#if defined(GTE_NOMINMAX_WAS_SET)
#undef NOMINMAX
#undef GTE_NOMINMAX_WAS_SET
#endif

#include <Applications/WindowApplication.h>
#include <Graphics/GraphicsEngine.h>

namespace gte
{
    class Window : public WindowApplication
    {
    public:
        struct Parameters : public WindowApplication::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle,
                int32_t inXOrigin, int32_t inYOrigin, int32_t inXSize, int32_t inYSize);

            HWND handle;
            HWND parent;
            bool hscrollBar, vscrollBar;

            // For DX11, the device creation flags are passed to the function
            // D3D11CreateDevice during construction of a DX11Engine object.
            // See the documentation for D3D11CreateDevice for the available
            // flags. For GL45, set the flags to 0 for the default behavior;
            // no additional semantics occur on GL45Engine construction. Set
            // bit 0 of the flag to 1 to tell the GL45Engine construction to
            // write a text file that contains the OpenGL driver information.
            // The default value is 0.  When bit 0 is set to 1, a text file
            // named OpenGLDriverInfo.txt is generated that contains the
            // OpenGL driver information. Other bit flags may be defined at
            // a later date.
            UINT deviceCreationFlags;
        };

        // Abstract base class. Only WindowSystem may create windows.
        virtual ~Window();

        // Member access.
        inline HWND GetHandle() const
        {
            return mHandle;
        }

        virtual void SetTitle(std::wstring const& title) override;

        // Mouse position information.
        virtual void SetMousePosition(int32_t x, int32_t y) override;
        virtual void GetMousePosition(int32_t& x, int32_t& y) const override;

        // Actions to take before the window closes.
        virtual void OnClose() override;

        // Scroll support. The 'bar' value is 0 for horizontal scroll bars
        // or 1 for vertical scroll bars.
        void SetScrollInterval(int32_t bar, int32_t minValue, int32_t maxValue);
        void GetScrollInterval(int32_t bar, int32_t& minValue, int32_t& maxValue) const;
        int32_t SetScrollPosition(int32_t bar, int32_t value);
        int32_t GetScrollPosition(int32_t bar) const;

        // The return value of the increment/decrement functions is the delta
        // of the slider thumb. If zero, the scroll state did not change.
        // For the tracking functions, the return value is the current slider
        // thumb position. A derived-class override must call the base-class
        // function first.
        int32_t OnScrollIncrementLoRes(int32_t bar);
        int32_t OnScrollDecrementLoRes(int32_t bar);
        int32_t OnScrollIncrementHiRes(int32_t bar);
        int32_t OnScrollDecrementHiRes(int32_t bar);
        int32_t OnScrollTracking(int32_t bar);
        int32_t OnScrollEndTracking(int32_t bar);

    protected:
        Window(Parameters& parameters);

        // Standard window information.
        HWND mHandle;

        // Scroll bar support.
        bool mHasScroll[2];
        mutable SCROLLINFO mScrollInfo[2];
        int32_t mScrollLoResDelta[2];
        int32_t mScrollHiResDelta[2];

        // TODO: This is assigned mBaseEngine, which allows development of
        // the DX12 engine independently of DX11 and WGL. The DX12 engine
        // is a work in progress.
        std::shared_ptr<GraphicsEngine> mEngine;
    };
}

// Window and WindowSystem have a circular dependency that cannot be broken
// by forward declarations in either header. The includion of the following
// header file at this location breaks the cycle, because Window is defined
// previously in this file and is known to the compiler when it includes this
// file.
#include <Applications/MSW/WindowSystem.h>
