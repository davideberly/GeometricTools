// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11Texture.h>
using namespace gte;

DX11Texture::~DX11Texture()
{
    DX11::FinalRelease(mSRView);
    DX11::FinalRelease(mUAView);
}

DX11Texture::DX11Texture(Texture const* gtTexture)
    :
    DX11Resource(gtTexture),
    mSRView(nullptr),
    mUAView(nullptr)
{
}

bool DX11Texture::Update(ID3D11DeviceContext* context, uint32_t sri)
{
    Texture* texture = GetTexture();
    LogAssert(sri < texture->GetNumSubresources(), "Subresource index out of range.");
    LogAssert(texture->GetUsage() == Resource::Usage::DYNAMIC_UPDATE, "Texture must be dynamic-update.");

    // Map the texture.
    ID3D11Resource* dxTexture = GetDXResource();
    D3D11_MAPPED_SUBRESOURCE sub;
    DX11Log(context->Map(dxTexture, sri, D3D11_MAP_WRITE_DISCARD, 0, &sub));

    // Copy from CPU memory.
    auto sr = texture->GetSubresource(sri);
    uint32_t numDimensions = texture->GetNumDimensions();
    if (numDimensions == 1)
    {
        std::memcpy(sub.pData, sr.data, texture->GetNumBytesFor(sr.level));
    }
    else if (numDimensions == 2)
    {
        CopyPitched2(texture->GetDimensionFor(sr.level, 1), sr.rowPitch,
            sr.data, sub.RowPitch, sub.pData);
    }
    else  // numDimensions == 3
    {
        CopyPitched3(texture->GetDimensionFor(sr.level, 1),
            texture->GetDimensionFor(sr.level, 2), sr.rowPitch, sr.slicePitch,
            sr.data, sub.RowPitch, sub.DepthPitch, sub.pData);
    }
    context->Unmap(dxTexture, sri);
    return true;
}

bool DX11Texture::Update(ID3D11DeviceContext* context)
{
    Texture* texture = GetTexture();
    uint32_t const numSubresources = texture->GetNumSubresources();
    for (uint32_t index = 0; index < numSubresources; ++index)
    {
        if (!Update(context, index))
        {
            return false;
        }
    }
    return true;
}

bool DX11Texture::CopyCpuToGpu(ID3D11DeviceContext* context, uint32_t sri)
{
    Texture* texture = GetTexture();
    LogAssert(sri < texture->GetNumSubresources(), "Subresource index out of range.");
    PreparedForCopy(D3D11_CPU_ACCESS_WRITE);

    // Map the staging texture.
    D3D11_MAPPED_SUBRESOURCE sub;
    DX11Log(context->Map(mStaging, sri, D3D11_MAP_WRITE, 0, &sub));

    // Copy from CPU memory to staging texture.
    auto sr = texture->GetSubresource(sri);
    uint32_t numDimensions = texture->GetNumDimensions();
    if (numDimensions == 1)
    {
        std::memcpy(sub.pData, sr.data, texture->GetNumBytesFor(sr.level));
    }
    else if (numDimensions == 2)
    {
        CopyPitched2(texture->GetDimensionFor(sr.level, 1), sr.rowPitch,
            sr.data, sub.RowPitch, sub.pData);
    }
    else  // numDimensions == 3
    {
        CopyPitched3(texture->GetDimensionFor(sr.level, 1),
            texture->GetDimensionFor(sr.level, 2), sr.rowPitch,
            sr.slicePitch, sr.data, sub.RowPitch, sub.DepthPitch,
            sub.pData);
    }
    context->Unmap(mStaging, sri);

    // Copy from staging texture to GPU memory.
    ID3D11Resource* dxTexture = GetDXResource();
    context->CopySubresourceRegion(dxTexture, sri, 0, 0, 0, mStaging, sri, nullptr);
    return true;
}

bool DX11Texture::CopyCpuToGpu(ID3D11DeviceContext* context)
{
    Texture* texture = GetTexture();
    uint32_t const numSubresources = texture->GetNumSubresources();
    for (uint32_t index = 0; index < numSubresources; ++index)
    {
        if (!CopyCpuToGpu(context, index))
        {
            return false;
        }
    }

    // Generate mipmaps (when they exist).
    if (texture->WantAutogenerateMipmaps() && mSRView)
    {
        context->GenerateMips(mSRView);
    }
    return true;
}

bool DX11Texture::CopyGpuToCpu(ID3D11DeviceContext* context, uint32_t sri)
{
    Texture* texture = GetTexture();
    LogAssert(sri < texture->GetNumSubresources(), "Subresource index out of range.");
    PreparedForCopy(D3D11_CPU_ACCESS_READ);

    // Copy from GPU memory to staging texture.
    ID3D11Resource* dxTexture = GetDXResource();
    context->CopySubresourceRegion(mStaging, sri, 0, 0, 0, dxTexture, sri, nullptr);

    // Map the staging texture.
    D3D11_MAPPED_SUBRESOURCE sub;
    DX11Log(context->Map(mStaging, sri, D3D11_MAP_READ, 0, &sub));

    // Copy from staging texture to CPU memory.
    auto sr = texture->GetSubresource(sri);
    uint32_t numDimensions = texture->GetNumDimensions();
    if (numDimensions == 1)
    {
        std::memcpy(sr.data, sub.pData, texture->GetNumBytesFor(sr.level));
    }
    else if (numDimensions == 2)
    {
        CopyPitched2(texture->GetDimensionFor(sr.level, 1), sub.RowPitch,
            sub.pData, sr.rowPitch, sr.data);
    }
    else  // numDimensions == 3
    {
        CopyPitched3(texture->GetDimensionFor(sr.level, 1),
            texture->GetDimensionFor(sr.level, 2), sub.RowPitch,
            sub.DepthPitch, sub.pData, sr.rowPitch, sr.slicePitch, sr.data);
    }
    context->Unmap(mStaging, sri);
    return true;
}

bool DX11Texture::CopyGpuToCpu(ID3D11DeviceContext* context)
{
    Texture* texture = GetTexture();
    uint32_t const numSubresources = texture->GetNumSubresources();
    for (uint32_t index = 0; index < numSubresources; ++index)
    {
        if (!CopyGpuToCpu(context, index))
        {
            return false;
        }
    }
    return true;
}

void DX11Texture::CopyGpuToGpu(ID3D11DeviceContext* context,
    ID3D11Resource* target, uint32_t sri)
{
    Texture* texture = GetTexture();
    LogAssert(sri < texture->GetNumSubresources(), "Subresource index out of range.");

    // Copy from GPU memory to staging texture.
    ID3D11Resource* dxTexture = GetDXResource();
    context->CopySubresourceRegion(target, sri, 0, 0, 0, dxTexture, sri, nullptr);
}

void DX11Texture::CopyGpuToGpu(ID3D11DeviceContext* context, ID3D11Resource* target)
{
    Texture* texture = GetTexture();
    uint32_t const numSubresources = texture->GetNumSubresources();
    for (uint32_t index = 0; index < numSubresources; ++index)
    {
        CopyGpuToGpu(context, target, index);
    }
}

void DX11Texture::SetName(std::string const& name)
{
    DX11Resource::SetName(name);
    DX11Log(DX11::SetPrivateName(mSRView, name));
    DX11Log(DX11::SetPrivateName(mUAView, name));
}

void DX11Texture::CopyPitched2(uint32_t numRows, uint32_t srcRowPitch,
    void const* srcData, uint32_t trgRowPitch, void* trgData)
{
    if (srcRowPitch == trgRowPitch)
    {
        // The memory is contiguous.
        std::memcpy(trgData, srcData,
            static_cast<size_t>(trgRowPitch) * static_cast<size_t>(numRows));
    }
    else
    {
        // Padding was added to each row of the texture, so we must
        // copy a row at a time to compensate for differing pitches.
        uint32_t numRowBytes = std::min(srcRowPitch, trgRowPitch);
        char const* srcRow = static_cast<char const*>(srcData);
        char* trgRow = static_cast<char*>(trgData);
        for (uint32_t row = 0; row < numRows; ++row)
        {
            std::memcpy(trgRow, srcRow, numRowBytes);
            srcRow += srcRowPitch;
            trgRow += trgRowPitch;
        }
    }
}

void DX11Texture::CopyPitched3(uint32_t numRows,
    uint32_t numSlices, uint32_t srcRowPitch,
    uint32_t srcSlicePitch, void const* srcData, uint32_t trgRowPitch,
    uint32_t trgSlicePitch, void* trgData)
{
    if (srcRowPitch == trgRowPitch && srcSlicePitch == trgSlicePitch)
    {
        // The memory is contiguous.
        std::memcpy(trgData, srcData,
            static_cast<size_t>(trgSlicePitch) * static_cast<size_t>(numSlices));
    }
    else
    {
        // Padding was added to each row and/or slice of the texture, so
        // we must copy the data to compensate for differing pitches.
        uint32_t numRowBytes = std::min(srcRowPitch, trgRowPitch);
        char const* srcSlice = static_cast<char const*>(srcData);
        char* trgSlice = static_cast<char*>(trgData);
        for (uint32_t slice = 0; slice < numSlices; ++slice)
        {
            char const* srcRow = srcSlice;
            char* trgRow = trgSlice;
            for (uint32_t row = 0; row < numRows; ++row)
            {
                std::memcpy(trgRow, srcRow, numRowBytes);
                srcRow += srcRowPitch;
                trgRow += trgRowPitch;
            }
            srcSlice += srcSlicePitch;
            trgSlice += trgSlicePitch;
        }
    }
}
