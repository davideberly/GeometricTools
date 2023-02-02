// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/GraphicsEngine.h>
#include <Mathematics/Logger.h>
using namespace gte;

GraphicsEngine::GraphicsEngine()
    :
    mCreateGEDrawTarget(nullptr),
    mGEObjectCreator(nullptr),
    mAllowOcclusionQuery(false),
    mWarnOnNonemptyBridges(true)
{
    mCreateGEObject.fill(nullptr);

    mGOListener = std::make_shared<GOListener>(this);
    GraphicsObject::SubscribeForDestruction(mGOListener);

    mDTListener = std::make_shared<DTListener>(this);
    DrawTarget::SubscribeForDestruction(mDTListener);
}

void GraphicsEngine::SetFont(std::shared_ptr<Font> const& font)
{
    LogAssert(font != nullptr, "Input font is null.");
    if (font != mActiveFont)
    {
        // Destroy font resources in GPU memory.  The mActiveFont should
        // be null once, only when the mDefaultFont is created.
        if (mActiveFont)
        {
            Unbind(mActiveFont->GetVertexBuffer());
            Unbind(mActiveFont->GetIndexBuffer());
            Unbind(mActiveFont->GetTextEffect()->GetTranslate());
            Unbind(mActiveFont->GetTextEffect()->GetColor());
            Unbind(mActiveFont->GetTextEffect()->GetVertexShader());
            Unbind(mActiveFont->GetTextEffect()->GetPixelShader());
        }

        mActiveFont = font;

        // Create font resources in GPU memory.
        Bind(mActiveFont->GetVertexBuffer());
        Bind(mActiveFont->GetIndexBuffer());
        Bind(mActiveFont->GetTextEffect()->GetTranslate());
        Bind(mActiveFont->GetTextEffect()->GetColor());
        Bind(mActiveFont->GetTextEffect()->GetVertexShader());
        Bind(mActiveFont->GetTextEffect()->GetPixelShader());
    }
}

uint64_t GraphicsEngine::Draw(Visual* visual)
{
    LogAssert(visual != nullptr, "Input visual is null.");
    auto const& vbuffer = visual->GetVertexBuffer();
    auto const& ibuffer = visual->GetIndexBuffer();
    auto const& effect = visual->GetEffect();
    if (vbuffer && ibuffer && effect)
    {
        return DrawPrimitive(vbuffer, ibuffer, effect);
    }
    return 0;
}

uint64_t GraphicsEngine::Draw(std::vector<Visual*> const& visuals)
{
    uint64_t numPixelsDrawn = 0;
    for (auto const& visual : visuals)
    {
        numPixelsDrawn += Draw(visual);
    }
    return numPixelsDrawn;
}

uint64_t GraphicsEngine::Draw(std::shared_ptr<Visual> const& visual)
{
    return Draw(visual.get());
}

uint64_t GraphicsEngine::Draw(std::vector<std::shared_ptr<Visual>> const& visuals)
{
    uint64_t numPixelsDrawn = 0;
    for (auto const& visual : visuals)
    {
        numPixelsDrawn += Draw(visual);
    }
    return numPixelsDrawn;
}

uint64_t GraphicsEngine::Draw(int32_t x, int32_t y, std::array<float, 4> const& color, std::string const& message)
{
    uint64_t numPixelsDrawn;

    if (message.length() > 0)
    {
        int32_t vx, vy, vw, vh;
        GetViewport(vx, vy, vw, vh);
        mActiveFont->Typeset(vw, vh, x, y, color, message);

        Update(mActiveFont->GetTextEffect()->GetTranslate());
        Update(mActiveFont->GetTextEffect()->GetColor());
        Update(mActiveFont->GetVertexBuffer());

        // We need to restore default state for text drawing.  Remember the
        // current state so that we can reset it after drawing.
        std::shared_ptr<BlendState> bState = GetBlendState();
        std::shared_ptr<DepthStencilState> dState = GetDepthStencilState();
        std::shared_ptr<RasterizerState> rState = GetRasterizerState();
        SetDefaultBlendState();
        SetDefaultDepthStencilState();
        SetDefaultRasterizerState();

        numPixelsDrawn = DrawPrimitive(mActiveFont->GetVertexBuffer(),
            mActiveFont->GetIndexBuffer(), mActiveFont->GetTextEffect());

        SetBlendState(bState);
        SetDepthStencilState(dState);
        SetRasterizerState(rState);
    }
    else
    {
        numPixelsDrawn = 0;
    }

    return numPixelsDrawn;
}

uint64_t GraphicsEngine::Draw(std::shared_ptr<OverlayEffect> const& overlay)
{
    LogAssert(overlay != nullptr, "Input overlay is null.");
    auto const& vbuffer = overlay->GetVertexBuffer();
    auto const& ibuffer = overlay->GetIndexBuffer();
    auto const& effect = overlay->GetEffect();
    if (vbuffer && ibuffer && effect)
    {
        return DrawPrimitive(vbuffer, ibuffer, effect);
    }
    return 0;
}

GEObject* GraphicsEngine::Bind(std::shared_ptr<GraphicsObject> const& object)
{
    LogAssert(object != nullptr, "Attempt to bind a null object.");

    mGOMapMutex.lock();
    GraphicsObject const* gtObject = object.get();
    GEObject* geObjectPtr = nullptr;
    auto iter = mGOMap.find(gtObject);
    if (iter == mGOMap.end())
    {
        // The 'create' function is not null with the current engine design.
        // If the assertion is triggered, someone changed the hierarchy of
        // GraphicsObjectType but did not change msCreateFunctions[] to match.
        CreateGEObject create = mCreateGEObject[object->GetType()];
        if (create)
        {
            auto geObject = create(mGEObjectCreator, gtObject);
            LogAssert(geObject != nullptr, "Unexpected condition.");

            iter = mGOMap.insert(std::make_pair(gtObject, geObject)).first;
#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
            geObject->SetName(object->GetName());
#endif
            geObjectPtr = iter->second.get();
        }
        // else: No logger message is generated here because GL4 does not have
        // shader creation functions.
    }
    else
    {
        geObjectPtr = iter->second.get();
    }
    mGOMapMutex.unlock();
    return geObjectPtr;
}

GEDrawTarget* GraphicsEngine::Bind(std::shared_ptr<DrawTarget> const& target)
{
    mDTMapMutex.lock();
    DrawTarget const* gtTarget = target.get();
    auto iter = mDTMap.find(gtTarget);
    if (iter == mDTMap.end())
    {
        uint32_t const numTargets = target->GetNumTargets();
        std::vector<GEObject*> rtTextures(numTargets);
        for (uint32_t i = 0; i < numTargets; ++i)
        {
            rtTextures[i] = static_cast<GEObject*>(Bind(target->GetRTTexture(i)));
        }

        std::shared_ptr<TextureDS> object = target->GetDSTexture();
        GEObject* dsTexture;
        if (object)
        {
            dsTexture = static_cast<GEObject*>(Bind(object));
        }
        else
        {
            dsTexture = nullptr;
        }

        auto geTarget = mCreateGEDrawTarget(gtTarget, rtTextures, dsTexture);
        LogAssert(geTarget != nullptr, "Unexpected condition.");

        iter = mDTMap.insert(std::make_pair(gtTarget, geTarget)).first;
    }
    GEDrawTarget* geDrawTargetPtr = iter->second.get();
    mDTMapMutex.unlock();
    return geDrawTargetPtr;
}

GEObject* GraphicsEngine::Get(std::shared_ptr<GraphicsObject> const& object) const
{
    mGOMapMutex.lock();
    GraphicsObject const* gtObject = object.get();
    GEObject* geObject = nullptr;
    auto iter = mGOMap.find(gtObject);
    if (iter != mGOMap.end())
    {
        geObject = iter->second.get();
    }
    mGOMapMutex.unlock();
    return geObject;
}

GEDrawTarget* GraphicsEngine::Get(std::shared_ptr<DrawTarget> const& target) const
{
    mDTMapMutex.lock();
    DrawTarget const* gtTarget = target.get();
    GEDrawTarget* geDrawTarget = nullptr;
    auto iter = mDTMap.find(gtTarget);
    if (iter != mDTMap.end())
    {
        geDrawTarget = iter->second.get();
    }
    mDTMapMutex.unlock();
    return geDrawTarget;
}

void GraphicsEngine::GetTotalAllocation(size_t& numBytes, size_t& numObjects) const
{
    mGOMapMutex.lock();
    numBytes = 0;
    numObjects = 0;
    for (auto const& element : mGOMap)
    {
        auto const& object = element.second;
        if (object)
        {
            auto resource = dynamic_cast<Resource*>(object->GetGraphicsObject());
            if (resource)
            {
                ++numObjects;
                numBytes += resource->GetNumBytes();
            }
        }
    }
    mGOMapMutex.unlock();
}

void GraphicsEngine::DestroyDefaultGlobalState()
{
    if (mDefaultBlendState)
    {
        Unbind(mDefaultBlendState);
    }

    if (mDefaultDepthStencilState)
    {
        Unbind(mDefaultDepthStencilState);
    }

    if (mDefaultRasterizerState)
    {
        Unbind(mDefaultRasterizerState);
    }

    BaseEngine::DestroyDefaultGlobalState();
}

bool GraphicsEngine::Unbind(GraphicsObject const* object)
{
    mGOMapMutex.lock();
    bool success = false;
    auto iter = mGOMap.find(object);
    if (iter != mGOMap.end())
    {
        uint32_t type = object->GetType();
        if (type == GT_VERTEX_BUFFER)
        {
            mILMap->Unbind(static_cast<VertexBuffer const*>(object));
        }
        else if (type == GT_VERTEX_SHADER)
        {
            mILMap->Unbind(static_cast<Shader const*>(object));
        }

        mGOMap.erase(iter);
        success = true;
    }
    mGOMapMutex.unlock();
    return success;
}

bool GraphicsEngine::Unbind(DrawTarget const* target)
{
    mDTMapMutex.lock();
    bool success = false;
    auto iter = mDTMap.find(target);
    if (iter != mDTMap.end())
    {
        mDTMap.erase(iter);
        success = true;
    }
    mDTMapMutex.unlock();
    return success;
}

GraphicsEngine::GOListener::GOListener(GraphicsEngine* engine)
    :
    mEngine(engine)
{
}

void GraphicsEngine::GOListener::OnDestroy(GraphicsObject const* object)
{
    if (mEngine)
    {
        mEngine->Unbind(object);
    }
}

GraphicsEngine::DTListener::DTListener(GraphicsEngine* engine)
    :
    mEngine(engine)
{
}

void GraphicsEngine::DTListener::OnDestroy(DrawTarget const* target)
{
    if (mEngine)
    {
        mEngine->Unbind(target);
    }
}
