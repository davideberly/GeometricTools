// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Vector4.h>
#include <Graphics/DrawingState.h>
#include <cstdint>

namespace gte
{
    class SamplerState : public DrawingState
    {
    public:
        // The encoding involves minification (MIN), magnification (MAG), and
        // mip-level filtering (MIP).  After each is P (POINT) or L (LINEAR).
        enum Filter
        {
            MIN_P_MAG_P_MIP_P,
            MIN_P_MAG_P_MIP_L,
            MIN_P_MAG_L_MIP_P,
            MIN_P_MAG_L_MIP_L,
            MIN_L_MAG_P_MIP_P,
            MIN_L_MAG_P_MIP_L,
            MIN_L_MAG_L_MIP_P,
            MIN_L_MAG_L_MIP_L,
            ANISOTROPIC,
            COMPARISON_MIN_P_MAG_P_MIP_P,
            COMPARISON_MIN_P_MAG_P_MIP_L,
            COMPARISON_MIN_P_MAG_L_MIP_P,
            COMPARISON_MIN_P_MAG_L_MIP_L,
            COMPARISON_MIN_L_MAG_P_MIP_P,
            COMPARISON_MIN_L_MAG_P_MIP_L,
            COMPARISON_MIN_L_MAG_L_MIP_P,
            COMPARISON_MIN_L_MAG_L_MIP_L,
            COMPARISON_ANISOTROPIC
        };

        // Modes for handling texture coordinates at texture-image boundaries.
        enum Mode
        {
            WRAP,
            MIRROR,
            CLAMP,
            BORDER,
            MIRROR_ONCE
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

        SamplerState();

        // Member access.  The members are intended to be write-once before
        // you create an associated graphics state.
        Filter filter;              // default: MIN_P_MAG_P_MIP_P
        std::array<Mode, 3> mode;   // default: { CLAMP, CLAMP, CLAMP }
        float mipLODBias;           // default: 0
        uint32_t maxAnisotropy;     // default: 1
        Comparison comparison;      // default: NEVER
        Vector4<float> borderColor; // default: { 0, 0, 0, 0 }
        float minLOD;               // default: -FLT_MAX
        float maxLOD;               // default: +FLT_MAX

    public:
        // For use by the Shader class for storing reflection information.
        static int32_t constexpr shaderDataLookup = 6;
    };
}
