// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DepthStencilState.h>
using namespace gte;

DepthStencilState::DepthStencilState()
    :
    depthEnable(true),
    writeMask(WriteMask::ALL),
    comparison(Comparison::LESS_EQUAL),
    stencilEnable(false),
    stencilReadMask(0xFF),
    stencilWriteMask(0xFF),
    reference(0)
{
    mType = GT_DEPTH_STENCIL_STATE;
}

