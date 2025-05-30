// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/SamplerState.h>
using namespace gte;

SamplerState::SamplerState()
    :
    filter(Filter::MIN_P_MAG_P_MIP_P),
    mode{ Mode::CLAMP, Mode::CLAMP, Mode::CLAMP },
    mipLODBias(0.0f),
    maxAnisotropy(1),
    comparison(Comparison::NEVER),
    borderColor{ 1.0f, 1.0f, 1.0f, 1.0f },
    minLOD(-std::numeric_limits<float>::max()),
    maxLOD(std::numeric_limits<float>::max())
{
    mType = GT_SAMPLER_STATE;
}

