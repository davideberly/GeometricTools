// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/BlendState.h>
using namespace gte;

BlendState::BlendState()
    :
    enableAlphaToCoverage(false),
    enableIndependentBlend(false),
    target{},
    blendColor{ 0.0f, 0.0f, 0.0f, 0.0f },
    sampleMask(0xFFFFFFFFu)
{
    mType = GT_BLEND_STATE;
}
