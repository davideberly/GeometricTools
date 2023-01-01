// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/DX11/DX11.h>
#include <cstdint>
#include <fstream>

// The renaming of the D3D11 items will be necessary when the DX12 engine
// is finished.  DX12 uses '12' in its names, of course.
#define ID3DShaderReflection ID3D11ShaderReflection
#define IID_ID3DShaderReflection IID_ID3D11ShaderReflection
#define ID3DShaderReflectionConstantBuffer ID3D11ShaderReflectionConstantBuffer
#define ID3DShaderReflectionType ID3D11ShaderReflectionType
#define ID3DShaderReflectionVariable ID3D11ShaderReflectionVariable
#define D3D_SHADER_DESC D3D11_SHADER_DESC
#define D3D_SIGNATURE_PARAMETER_DESC D3D11_SIGNATURE_PARAMETER_DESC
#define D3D_SHADER_VERSION_TYPE D3D11_SHADER_VERSION_TYPE
#define D3D_SHVER_GET_TYPE D3D11_SHVER_GET_TYPE
#define D3D_SHVER_GET_MAJOR D3D11_SHVER_GET_MAJOR
#define D3D_SHVER_GET_MINOR D3D11_SHVER_GET_MINOR
#define D3D_SHADER_BUFFER_DESC D3D11_SHADER_BUFFER_DESC
#define D3D_SHADER_INPUT_BIND_DESC D3D11_SHADER_INPUT_BIND_DESC
#define D3D_SHADER_VARIABLE_DESC D3D11_SHADER_VARIABLE_DESC
#define D3D_SHADER_TYPE_DESC D3D11_SHADER_TYPE_DESC

namespace gte
{
    class HLSLResource
    {
    public:
        // Abstract base class, destructor.
        virtual ~HLSLResource() = default;
    protected:
        // Construction.
        HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t numBytes);
        HLSLResource(D3D_SHADER_INPUT_BIND_DESC const& desc, uint32_t index, uint32_t numBytes);

    public:
        struct Description
        {
            Description()
                :
                name(""),
                type(D3D_SIT_CBUFFER),
                bindPoint(0),
                bindCount(0),
                flags(0),
                returnType(D3D_RETURN_TYPE_UNORM),
                dimension(D3D_SRV_DIMENSION_UNKNOWN),
                numSamples(0)
            {
            }

            std::string name;
            D3D_SHADER_INPUT_TYPE type;
            uint32_t bindPoint;
            uint32_t bindCount;
            uint32_t flags;
            D3D_RESOURCE_RETURN_TYPE returnType;
            D3D_SRV_DIMENSION dimension;
            uint32_t numSamples;
        };

        // Member access.
        inline std::string const& GetName() const
        {
            return mDesc.name;
        }

        inline D3D_SHADER_INPUT_TYPE GetType() const
        {
            return mDesc.type;
        }

        inline uint32_t GetBindPoint() const
        {
            return mDesc.bindPoint;
        }

        inline uint32_t GetBindCount() const
        {
            return mDesc.bindCount;
        }

        inline uint32_t GetFlags() const
        {
            return mDesc.flags;
        }

        inline D3D_RESOURCE_RETURN_TYPE GetReturnType() const
        {
            return mDesc.returnType;
        }

        inline D3D_SRV_DIMENSION GetDimension() const
        {
            return mDesc.dimension;
        }

        inline uint32_t GetNumSamples() const
        {
            return mDesc.numSamples;
        }

        inline uint32_t GetNumBytes() const
        {
            return mNumBytes;
        }

        // Print to a text file for human readability.
        virtual void Print(std::ofstream& output) const;

    private:
        Description mDesc;
        uint32_t mNumBytes;

        // Support for Print.
        static std::string const msSIType[];
        static std::string const msReturnType[];
        static std::string const msSRVDimension[];
    };
}
