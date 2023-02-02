// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/RasterizerState.h>
using namespace gte;

RasterizerState::RasterizerState()
    :
    fill(Fill::SOLID),
    cull(Cull::BACK),
    frontCCW(true),
    depthBias(0),
    depthBiasClamp(0.0f),
    slopeScaledDepthBias(0.0f),
    enableDepthClip(true),
    enableScissor(false),
    enableMultisample(false),
    enableAntialiasedLine(false)
{
    mType = GT_RASTERIZER_STATE;
}
