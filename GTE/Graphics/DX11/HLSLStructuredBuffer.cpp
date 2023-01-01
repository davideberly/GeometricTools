// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLStructuredBuffer.h>
using namespace gte;

HLSLStructuredBuffer::HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc)
    :
    HLSLResource(desc, 0)
{
    Initialize(desc);
}

HLSLStructuredBuffer::HLSLStructuredBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index)
    :
    HLSLResource(desc, index, 0)
{
    Initialize(desc);
}

void HLSLStructuredBuffer::Initialize(D3D_SHADER_INPUT_BIND_DESC const& desc)
{
    if (desc.Type == D3D_SIT_STRUCTURED
    ||  desc.Type == D3D_SIT_UAV_RWSTRUCTURED)
    {
        mType = Type::SBT_BASIC;
    }
    else if (desc.Type == D3D_SIT_UAV_APPEND_STRUCTURED)
    {
        mType = Type::SBT_APPEND;
    }
    else if (desc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED)
    {
        mType = Type::SBT_CONSUME;
    }
    else if (desc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
    {
        mType = Type::SBT_COUNTER;
    }
    else
    {
        mType = Type::SBT_INVALID;
    }

    mGpuWritable =
        desc.Type == D3D_SIT_UAV_RWSTRUCTURED ||
        desc.Type == D3D_SIT_UAV_APPEND_STRUCTURED ||
        desc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED ||
        desc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER;
}
