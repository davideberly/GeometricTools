// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLByteAddressBuffer.h>
using namespace gte;

HLSLByteAddressBuffer::HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc)
    :
    HLSLResource(desc, 0),
    mGpuWritable(desc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
{
}

HLSLByteAddressBuffer::HLSLByteAddressBuffer(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index)
    :
    HLSLResource(desc, index, 0),
    mGpuWritable(desc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
{
}
