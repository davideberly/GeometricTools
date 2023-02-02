// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLResourceBindInfo.h>
using namespace gte;

HLSLResourceBindInfo::HLSLResourceBindInfo(
    D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t numBytes,
    std::vector<Member> const& members)
    :
    HLSLBaseBuffer(desc, numBytes, members)
{
}

HLSLResourceBindInfo::HLSLResourceBindInfo(
    D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index,
    uint32_t numBytes, std::vector<Member> const& members)
    :
    HLSLBaseBuffer(desc, index, numBytes, members)
{
}
