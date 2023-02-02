// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

// <windows.h> is referenced by the DX11 headers.  Turn off the min and max
// macros to avoid conflicts with std::min and std::max.
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <DXGI.h>
#include <stdexcept>
#include <string>

// Required libraries for linking DX11.0 with GTEngine:
//   d3d11.lib          (for DX11.0 core)
//   d3dcompiler.lib    (for D3DCompile and D3DReflect)
//   dxgi.lib           (for IDXGIAdapter1 and IDXGIOutput)
//   dxguid.lib         (for IID_ID3D11ShaderReflection)

namespace gte
{
    class DX11
    {
    public:
        template <typename T>
        static ULONG SafeAddRef(T* object)
        {
            if (object)
            {
                ULONG refs = object->AddRef();
                return refs;
            }
            return 0;
        }

        template <typename T>
        static ULONG SafeRelease(T*& object)
        {
            if (object)
            {
                ULONG refs = object->Release();
                object = nullptr;
                return refs;
            }
            return 0;
        }

        template <typename T>
        static ULONG FinalRelease(T*& object)
        {
            if (object)
            {
                ULONG refs = object->Release();
                object = nullptr;
                if (refs > 0)
                {
                    throw std::runtime_error(std::string(__FILE__) + "(" + std::string(__FUNCTION__) + "," + std::to_string(__LINE__) + "): Reference count is not zero after release.\n");
                }
            }
            return 0;
        }

        template <typename T>
        static ULONG GetNumReferences(T* object)
        {
            if (object)
            {
                object->AddRef();
                ULONG refs = object->Release();
                return refs;
            }
            return 0;
        }

        static HRESULT SetPrivateName(ID3D11DeviceChild* object, std::string const& name);
        static HRESULT SetPrivateName(IDXGIObject* object, std::string const& name);
        static void Log(HRESULT hr, char const* file, char const* function, int32_t line);
    };
}


// Macro for logging and throwing exceptions on HRESULT errors.
#define DX11Log(x) \
{ \
HRESULT hresult_ = (x); \
gte::DX11::Log(hresult_, __FILE__, __func__, __LINE__); \
}


// Fake enumerations to have human-readable names that D3D11 did not provide.

// D3D11_BIND_FLAG
UINT constexpr D3D11_BIND_NONE = 0;

// D3D11_RESOURCE_MISC_FLAG
UINT constexpr D3D11_RESOURCE_MISC_NONE = 0;

// D3D11_BUFFER_UAV_FLAG
UINT constexpr D3D11_BUFFER_UAV_FLAG_BASIC = 0;

// D3D11_CPU_ACCESS_FLAG
UINT constexpr D3D11_CPU_ACCESS_NONE = 0;
UINT constexpr D3D11_CPU_ACCESS_READ_WRITE = (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);

// D3D11_QUERY_MISC_FLAG
UINT constexpr D3D11_QUERY_MISC_NONE = 0;
