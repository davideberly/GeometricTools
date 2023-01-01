// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.07.04

#pragma once

#include <Graphics/Shader.h>
#include <Graphics/VertexBuffer.h>
#include <Graphics/DX11/DX11.h>

namespace gte
{
    class DX11InputLayout
    {
    public:
        // Construction and destruction.
        ~DX11InputLayout();
        DX11InputLayout(ID3D11Device* device, VertexBuffer const* vbuffer, Shader const* vshader);

        // Support for drawing geometric primitives.
        void Enable(ID3D11DeviceContext* context);
        void Disable(ID3D11DeviceContext* context);

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        HRESULT SetName(std::string const& name);

        inline std::string const& GetName() const
        {
            return mName;
        }

    private:
        ID3D11InputLayout* mLayout;
        int32_t mNumElements;
        D3D11_INPUT_ELEMENT_DESC mElements[VAConstant::MAX_ATTRIBUTES];
        std::string mName;

        // Conversions from GTEngine values to DX11 values.
        static char const* msSemantic[VASemantic::NUM_SEMANTICS];
    };
}
