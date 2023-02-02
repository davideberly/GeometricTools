// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DrawingState.h>
#include <cstdint>

namespace gte
{
    class RasterizerState : public DrawingState
    {
    public:
        enum Fill
        {
            SOLID,
            WIREFRAME
        };

        enum Cull
        {
            NONE,
            FRONT,
            BACK
        };

        RasterizerState();

        // Member access.  The members are intended to be write-once before
        // you create an associated graphics state.
        Fill fill;                  // default: SOLID
        Cull cull;                  // default: BACK
        bool frontCCW;              // default: true
        int32_t depthBias;          // default: 0
        float depthBiasClamp;       // default: 0
        float slopeScaledDepthBias; // default: 0
        bool enableDepthClip;       // default: true
        bool enableScissor;         // default: false
        bool enableMultisample;     // default: false
        bool enableAntialiasedLine; // default: false
    };
}
