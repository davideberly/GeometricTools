// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/DrawTarget.h>
#include <Mathematics/Logger.h>
using namespace gte;

DrawTarget::~DrawTarget()
{
    msLFDMutex.lock();
    {
        for (auto const& listener : msLFDSet)
        {
            listener->OnDestroy(this);
        }
    }
    msLFDMutex.unlock();
}

DrawTarget::DrawTarget(uint32_t numRenderTargets, uint32_t rtFormat,
    uint32_t width, uint32_t height, bool hasRTMipmaps,
    bool createRTStorage, uint32_t dsFormat, bool createDSStorage)
{
    LogAssert(numRenderTargets > 0, "Number of targets must be at least one.");
    mRTTextures.resize(numRenderTargets);
    for (auto& texture : mRTTextures)
    {
        texture = std::make_shared<TextureRT>(rtFormat, width, height,
            hasRTMipmaps, createRTStorage);
    }

    if (dsFormat != DF_UNKNOWN)
    {
        if (DataFormat::IsDepth(dsFormat))
        {
            mDSTexture = std::make_shared<TextureDS>(dsFormat, width,
                height, createDSStorage);
            return;
        }
        LogError("Invalid depth-stencil format.");
    }
}

uint32_t DrawTarget::GetRTFormat() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetFormat();
}

uint32_t DrawTarget::GetWidth() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetWidth();
}

uint32_t DrawTarget::GetHeight() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->GetHeight();
}

bool DrawTarget::HasRTMipmaps() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->HasMipmaps();
}

uint32_t DrawTarget::GetDSFormat() const
{
    LogAssert(mDSTexture != nullptr, "Unexpected condition.");
    return mDSTexture->GetFormat();
}

std::shared_ptr<TextureRT> const DrawTarget::GetRTTexture(uint32_t i) const
{
    LogAssert(i < static_cast<uint32_t>(mRTTextures.size()), "Unexpected condition.");
    return mRTTextures[i];
}

void DrawTarget::AutogenerateRTMipmaps()
{
    if (HasRTMipmaps())
    {
        for (auto& texture : mRTTextures)
        {
            texture->AutogenerateMipmaps();
        }
    }
}

bool DrawTarget::WantAutogenerateRTMipmaps() const
{
    LogAssert(mRTTextures.size() > 0, "Unexpected condition.");
    return mRTTextures[0]->WantAutogenerateMipmaps();
}

void DrawTarget::SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.insert(listener);
    }
    msLFDMutex.unlock();
}

void DrawTarget::UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.erase(listener);
    }
    msLFDMutex.unlock();
}


std::mutex DrawTarget::msLFDMutex;
std::set<std::shared_ptr<DrawTarget::ListenerForDestruction>> DrawTarget::msLFDSet;
