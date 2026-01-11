// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/DrawingState.h>
#include <Graphics/GL46/GL46GraphicsObject.h>

namespace gte
{
    class GL46DrawingState : public GL46GraphicsObject
    {
    public:
        // Abstract base class.
        virtual ~GL46DrawingState() = default;
    protected:
        GL46DrawingState(DrawingState const* gtState);
    };
}


