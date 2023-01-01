// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11InputLayout.h>
using namespace gte;

DX11InputLayout::~DX11InputLayout()
{
    DX11::FinalRelease(mLayout);
}

DX11InputLayout::DX11InputLayout(ID3D11Device* device,
    VertexBuffer const* vbuffer, Shader const* vshader)
    :
    mLayout(nullptr),
    mNumElements(0)
{
    LogAssert(vbuffer != nullptr && vshader != nullptr, "Invalid inputs.");

    std::memset(&mElements[0], 0, VAConstant::MAX_ATTRIBUTES*sizeof(mElements[0]));

    VertexFormat const& format = vbuffer->GetFormat();
    mNumElements = format.GetNumAttributes();
    for (int32_t i = 0; i < mNumElements; ++i)
    {
        VASemantic semantic{};
        DFType type{};
        uint32_t unit{}, offset{};
        format.GetAttribute(i, semantic, type, unit, offset);

        D3D11_INPUT_ELEMENT_DESC& element = mElements[i];
        element.SemanticName = msSemantic[semantic];
        element.SemanticIndex = unit;
        element.Format = static_cast<DXGI_FORMAT>(type);
        element.InputSlot = 0;  // TODO: Streams not yet supported.
        element.AlignedByteOffset = offset;
        element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element.InstanceDataStepRate = 0;
    }

    auto const& compiledCode = vshader->GetCompiledCode();
    DX11Log(device->CreateInputLayout(mElements, (UINT)mNumElements,
        &compiledCode[0], compiledCode.size(), &mLayout));
}

void DX11InputLayout::Enable(ID3D11DeviceContext* context)
{
    if (mLayout)
    {
        context->IASetInputLayout(mLayout);
    }
}

void DX11InputLayout::Disable(ID3D11DeviceContext* context)
{
    if (mLayout)
    {
        // TODO: Verify that mLayout is the active input layout.
        context->IASetInputLayout(nullptr);
    }
}

HRESULT DX11InputLayout::SetName(std::string const& name)
{
    mName = name;
    return DX11::SetPrivateName(mLayout, mName);
}


char const* DX11InputLayout::msSemantic[VASemantic::NUM_SEMANTICS] =
{
    "",
    "POSITION",
    "BLENDWEIGHT",
    "BLENDINDICES",
    "NORMAL",
    "PSIZE",
    "TEXCOORD",
    "TANGENT",
    "BINORMAL",
    "TESSFACTOR",
    "POSITIONT",
    "COLOR",
    "FOG",
    "DEPTH",
    "SAMPLE"
};
