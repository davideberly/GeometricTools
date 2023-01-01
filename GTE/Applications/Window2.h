// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.07.04

#pragma once

#if defined(GTE_USE_MSWINDOWS)
#include <Applications/MSW/Window.h>
#endif
#if defined(GTE_USE_LINUX)
#include <Applications/GLX/Window.h>
#endif
#include <Graphics/DepthStencilState.h>
#include <Graphics/OverlayEffect.h>
#include <Graphics/Texture2.h>
#include <cstdint>

// The 2D application support is for GTEngine samples and is not a
// general-purpose 2D windowing layer.  The window resizing is disabled to
// avoid the complications of resizing the overlay texture and redrawing
// objects that were initially defined in the original window coordinates.

namespace gte
{
    class Window2 : public Window
    {
    protected:
        // Abstract base class.  The window creation requires
        // parameters.allowResize to be 'false'; otherwise, the creation
        // fails.
        Window2(Parameters& parameters);

    public:
        // Display functions.  

        // OnResize is called during window resize events. The screen texture
        // and the overlay objects are recreated. The engine then resizes its
        // renderer. If a class derived from Window2 wants resizing, it must
        // set the parameters.allowResize to true. Moreover, the class must
        // override OnResize, calling the Window2::OnResize first and then
        // calling either its own OnDisplay or OnIdle, whichever it is using
        // for drawing. This is required because during the window resizing,
        // the message pump is receiving the events and OnIdle() is not
        // called directly in the message pump because there are messages
        // that must be processed first. If you do not call your own OnIdle,
        // the rendered client window will not display properly.
        virtual bool OnResize(int32_t xSize, int32_t ySize) override;

        // The OnDisplay function updates the screen texture if it is 'dirty'
        // and then draws the 2D overlay followed by a call to
        // DrawScreenOverlay.
        virtual void OnDisplay() override;

        // The DrawScreenOverlay function is called after the screen texture
        // is drawn but before the swap-buffers call is made. This allows you
        // to draw text or user-created GUI elements on top of the screen
        // texture.
        virtual void DrawScreenOverlay();

        // Drawing functions.  Each color is packed as R8G8B8A8 with the alpha
        // channel the most significant bits and red the least significant
        // bits.  For example, (r,g,b,a) = (1,2,3,4) is 0x04030201.

        // Set all pixels to the specified color.
        void ClearScreen(uint32_t color);

        // Set the pixel at location (x,y) to the specified color.
        void SetPixel(int32_t x, int32_t y, uint32_t color);

        // Get the pixel color at location (x,y).
        uint32_t GetPixel(int32_t x, int32_t y);

        // Set the pixels (x',y') for x-thick <= x' <= x+thick and
        // y-thick <= y' <= y+thick.
        void DrawThickPixel(int32_t x, int32_t y, int32_t thick, uint32_t color);

        // Use Bresenham's algorithm to draw the line from (x0,y0) to (x1,y1)
        // using the specified color for the drawn pixels.  The algorithm is
        // biased in that the pixels set by DrawLine(x0,y0,x1,y1) are not
        // necessarily the same as those set by DrawLine(x1,y1,x0,y0).
        // TODO: Implement the midpoint algorithm to avoid the bias.
        void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
        void DrawThickLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t thick, uint32_t color);

        // Draw an axis-aligned rectangle using the specified color.  The
        // 'solid' parameter indicates whether or not to fill the rectangle.
        void DrawRectangle(int32_t xMin, int32_t yMin, int32_t xMax, int32_t yMax, uint32_t color, bool solid);
        void DrawThickRectangle(int32_t xMin, int32_t yMin, int32_t xMax, int32_t yMax, int32_t thick, uint32_t color, bool solid);

        // Use Bresenham's algorithm to draw the circle centered at
        // (xCenter,yCenter) with the specified 'radius' and using the
        // specified color.  The 'solid' parameter indicates whether or not
        // to fill the circle.
        void DrawCircle(int32_t xCenter, int32_t yCenter, int32_t radius, uint32_t color, bool solid);
        void DrawThickCircle(int32_t xCenter, int32_t yCenter, int32_t radius, int32_t thick, uint32_t color, bool solid);

        // Use Bresenham's algorithm to draw the axis-aligned ellipse
        // ((x-xc)/a)^2 + ((y-yc)/b)^2 = 1, where xCenter is xc, yCenter
        // is yc, xExtent is a, and yExtent is b.
        void DrawEllipse(int32_t xCenter, int32_t yCenter, int32_t xExtent, int32_t yExtent, uint32_t color);
        void DrawThickEllipse(int32_t xCenter, int32_t yCenter, int32_t xExtent, int32_t yExtent, int32_t thick, uint32_t color);

        // Flood-fill a region whose pixels are of color 'backColor' by
        // changing their color to 'foreColor'.  The fill treats the screen
        // as 4-connected; that is, after (x,y) is visited, then (x-1,y),
        // (x+1,y), (x,y-1), and (x,y+1) are visited (as long as they are in
        // the screen boundary).  The function simulates recursion by using
        // stacks, which avoids the expense of true recursion and the
        // potential to overflow the calling stack.
        void DrawFloodFill4(int32_t x, int32_t y, uint32_t foreColor, uint32_t backColor);

    protected:
        std::shared_ptr<OverlayEffect> mOverlay;
        std::shared_ptr<Texture2> mScreenTexture;
        std::shared_ptr<DepthStencilState> mNoDepthStencilState;
        std::function<void(int32_t, int32_t)> mDrawPixel;
        std::function<void(int32_t, int32_t)> mDrawThickPixel;
        uint32_t mPixelColor;
        int32_t mThick;
        bool mClampToWindow;
        bool mDoFlip;
        bool mScreenTextureNeedsUpdate;
    };
}
