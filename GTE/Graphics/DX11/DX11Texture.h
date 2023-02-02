// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Texture.h>
#include <Graphics/DX11/DX11Resource.h>
#include <cstdint>

namespace gte
{
    class DX11Texture : public DX11Resource
    {
    public:
        // Abstract base class.
        virtual ~DX11Texture();
    protected:
        DX11Texture(Texture const* gtTexture);

    public:
        // Member access.
        inline Texture* GetTexture() const
        {
            return static_cast<Texture*>(mGTObject);
        }

        inline ID3D11ShaderResourceView* GetSRView() const
        {
            return mSRView;
        }

        inline ID3D11UnorderedAccessView* GetUAView() const
        {
            return mUAView;
        }

        // Copy of data between CPU and GPU.
        virtual bool Update(ID3D11DeviceContext* context, uint32_t sri) override;
        virtual bool Update(ID3D11DeviceContext* context) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context, uint32_t sri) override;
        virtual bool CopyCpuToGpu(ID3D11DeviceContext* context) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context, uint32_t sri) override;
        virtual bool CopyGpuToCpu(ID3D11DeviceContext* context) override;
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context,
            ID3D11Resource* target, uint32_t sri) override;
        virtual void CopyGpuToGpu(ID3D11DeviceContext* context,
            ID3D11Resource* target) override;

        // Support for the DX11 debug layer; see comments in the file
        // DX11GraphicsObject.h about usage.
        virtual void SetName(std::string const& name) override;

    protected:
        // Support for copy of row-pitched and slice-pitched (noncontiguous)
        // texture memory.
        static void CopyPitched2(uint32_t numRows, uint32_t srcRowPitch,
            void const* srcData, uint32_t trgRowPitch, void* trgData);

        static void CopyPitched3(uint32_t numRows, uint32_t numSlices,
            uint32_t srcRowPitch, uint32_t srcSlicePitch,
            void const* srcData, uint32_t trgRowPitch,
            uint32_t trgSlicePitch, void* trgData);

        ID3D11ShaderResourceView* mSRView;
        ID3D11UnorderedAccessView* mUAView;
    };
}
