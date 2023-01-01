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
    class DepthStencilState : public DrawingState
    {
    public:
        enum WriteMask
        {
            ZERO,
            ALL
        };

        enum Comparison
        {
            NEVER,
            LESS,
            EQUAL,
            LESS_EQUAL,
            GREATER,
            NOT_EQUAL,
            GREATER_EQUAL,
            ALWAYS
        };

        enum Operation
        {
            OP_KEEP,
            OP_ZERO,
            OP_REPLACE,
            OP_INCR_SAT,
            OP_DECR_SAT,
            OP_INVERT,
            OP_INCR,
            OP_DECR
        };

        struct Face
        {
            Face()
                :
                fail(Operation::OP_KEEP),
                depthFail(Operation::OP_KEEP),
                pass(Operation::OP_KEEP),
                comparison(Comparison::ALWAYS)
            {
            }

            Operation fail;
            Operation depthFail;
            Operation pass;
            Comparison comparison;
        };

        DepthStencilState();

        // Member access.  The members are intended to be write-once before
        // you create an associated graphics state.
        bool depthEnable;           // default: true
        WriteMask writeMask;         // default: ALL
        Comparison comparison;        // default: LESS_EQUAL
        bool stencilEnable;         // default: false
        uint8_t stencilReadMask;    // default: 0xFF
        uint8_t stencilWriteMask;   // default: 0xFF
        Face frontFace;             // default: { KEEP, KEEP, KEEP, ALWAYS }
        Face backFace;              // default: { KEEP, KEEP, KEEP, ALWAYS }
        uint32_t reference;         // default: 0
    };
}
