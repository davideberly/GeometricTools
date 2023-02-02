// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.02.02

#pragma once

#include <Applications/Application.h>
#include <Applications/OnIdleTimer.h>

namespace gte
{
    class WindowApplication : public Application
    {
    public:
        struct Parameters : public Application::Parameters
        {
            Parameters();

            Parameters(std::wstring const& inTitle,
                int32_t inXOrigin, int32_t inYOrigin, int32_t inXSize, int32_t inYSize);

            std::wstring title;
            int32_t xOrigin, yOrigin, xSize, ySize;
            bool allowResize, useDepth24Stencil8, created;
        };

    public:
        // Abstract base class. Only WindowSystem may create windows.
        virtual ~WindowApplication() = default;
    protected:
        WindowApplication(Parameters& parameters);

    public:
        // Member access.
        virtual void SetTitle(std::wstring const& title)
        {
            mTitle = title;
        }

        inline std::wstring GetTitle() const
        {
            return mTitle;
        }

        inline int32_t GetXOrigin() const
        {
            return mXOrigin;
        }

        inline int32_t GetYOrigin() const
        {
            return mYOrigin;
        }

        inline int32_t GetXSize() const
        {
            return mXSize;
        }

        inline int32_t GetYSize() const
        {
            return mYSize;
        }

        inline bool IsMinimized() const
        {
            return mIsMinimized;
        }

        inline bool IsMaximized() const
        {
            return mIsMaximized;
        }

        inline float GetAspectRatio() const
        {
            return static_cast<float>(mXSize) / static_cast<float>(mYSize);
        }

        // Display callbacks.
        virtual void OnMove(int32_t x, int32_t y);
        virtual bool OnResize(int32_t xSize, int32_t ySize);
        virtual void OnMinimize();
        virtual void OnMaximize();
        virtual void OnDisplay();
        virtual void OnIdle();

        // Keyboard callbacks. OnCharPress allows you to distinguish between
        // upper-case and lower-case letters; OnKeyDown and OnKeyUp do not.
        // For OnCharPress, pressing KEY_ESCAPE terminates the application.
        // Pressing ' ' resets the application timer.
        virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y);
        virtual bool OnKeyDown(int32_t key, int32_t x, int32_t y);
        virtual bool OnKeyUp(int32_t key, int32_t x, int32_t y);

        // Mouse callbacks and state information.
        // TODO: HACK FOR NOW. Once these are removed, all the sample
        // applications must have their signatures changed.
        typedef int32_t MouseButton;
        typedef int32_t MouseState;
        // END TODO;
        virtual bool OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers);
        virtual bool OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers);
        virtual bool OnMouseWheel(int32_t delta, int32_t x, int32_t y, uint32_t modifiers);
        virtual void SetMousePosition(int32_t x, int32_t y);
        virtual void GetMousePosition(int32_t& x, int32_t& y) const;

#if defined(GTE_USE_MSWINDOWS)
        // Allow an application to pass data to another application.
        virtual void OnCopyData(HWND sender, PCOPYDATASTRUCT copyData);
#endif

        // Actions to take before the window closes.
        virtual void OnClose();

#if defined(GTE_USE_MSWINDOWS)
        // Allow an application to process the Windows message. The function
        // is called in WindowSystem::WindowProcedure. If the return value is
        // 'true', the message is not passed to 'window' via the message pump.
        // If the return value is 'false', the message is passed to 'window'.
        // This allows a third-party UI package to intercept messages and
        // process them as needed.
        virtual bool OnWindowsMessage(HWND handle, UINT message,
            WPARAM wParam, LPARAM lParam, LRESULT& result);
#endif

        // Key identifiers. These are platform-specific, so classes that
        // derived from WindowApplication must define these variables. They
        // are not defined by WindowApplication itself.
        static int32_t const KEY_ESCAPE;
        static int32_t const KEY_LEFT;
        static int32_t const KEY_RIGHT;
        static int32_t const KEY_UP;
        static int32_t const KEY_DOWN;
        static int32_t const KEY_HOME;
        static int32_t const KEY_END;
        static int32_t const KEY_PAGE_UP;
        static int32_t const KEY_PAGE_DOWN;
        static int32_t const KEY_INSERT;
        static int32_t const KEY_DELETE;
        static int32_t const KEY_F1;
        static int32_t const KEY_F2;
        static int32_t const KEY_F3;
        static int32_t const KEY_F4;
        static int32_t const KEY_F5;
        static int32_t const KEY_F6;
        static int32_t const KEY_F7;
        static int32_t const KEY_F8;
        static int32_t const KEY_F9;
        static int32_t const KEY_F10;
        static int32_t const KEY_F11;
        static int32_t const KEY_F12;
        static int32_t const KEY_BACKSPACE;
        static int32_t const KEY_TAB;
        static int32_t const KEY_ENTER;
        static int32_t const KEY_RETURN;

        // Keyboard modifiers.
        static int32_t const KEY_SHIFT;
        static int32_t const KEY_CONTROL;
        static int32_t const KEY_ALT;
        static int32_t const KEY_COMMAND;

        // Mouse buttons.
        static int32_t const MOUSE_NONE;
        static int32_t const MOUSE_LEFT;
        static int32_t const MOUSE_MIDDLE;
        static int32_t const MOUSE_RIGHT;

        // Mouse state.
        static int32_t const MOUSE_UP;
        static int32_t const MOUSE_DOWN;

        // Mouse modifiers.
        static int32_t const MODIFIER_CONTROL;
        static int32_t const MODIFIER_LBUTTON;
        static int32_t const MODIFIER_MBUTTON;
        static int32_t const MODIFIER_RBUTTON;
        static int32_t const MODIFIER_SHIFT;

    protected:
        // Standard window information.
        std::wstring mTitle;
        int32_t mXOrigin, mYOrigin, mXSize, mYSize;
        bool mAllowResize;
        bool mIsMinimized;
        bool mIsMaximized;

        OnIdleTimer mTimer;
    };
}
