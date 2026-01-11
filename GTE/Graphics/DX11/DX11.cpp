// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11.h>
#include <comdef.h>
#include <codecvt>
#include <locale>
using namespace gte;

HRESULT DX11::SetPrivateName(ID3D11DeviceChild* object, std::string const& name)
{
    HRESULT hr;
    if (object && name != "")
    {
        hr = object->SetPrivateData(WKPDID_D3DDebugObjectName,
            static_cast<UINT>(name.length()), name.c_str());
    }
    else
    {
        // Callers are allowed to call this function with a null input
        // or with an empty name (for convenience).
        hr = S_OK;
    }
    return hr;
}

HRESULT DX11::SetPrivateName(IDXGIObject* object, std::string const& name)
{
    HRESULT hr;
    if (object && name != "")
    {
        hr = object->SetPrivateData(WKPDID_D3DDebugObjectName,
            static_cast<UINT>(name.length()), name.c_str());
    }
    else
    {
        // Callers are allowed to call this function with a null input
        // or with an empty name (for convenience).
        hr = S_OK;
    }
    return hr;
}

void DX11::Log(HRESULT hr, char const* file, char const* function, int32_t line)
{
    if (FAILED(hr))
    {
        auto const* errorMessage = _com_error(hr).ErrorMessage();
        std::wstring input(errorMessage);
        std::int32_t numInput = static_cast<std::int32_t>(input.size());
        std::vector<char> output(numInput + 1, 0);
        WideCharToMultiByte(CP_ACP, 0, input.data(), numInput, output.data(), numInput + 1, NULL, NULL);
        output.back() = 0;
        std::string message = output.data();
        throw std::runtime_error(std::string(file) + "(" + std::string(function) + "," + std::to_string(line) + "): " + message + "\n");
    }
}


