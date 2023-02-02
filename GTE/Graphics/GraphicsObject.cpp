// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/GraphicsObject.h>
using namespace gte;

GraphicsObject::~GraphicsObject()
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

GraphicsObject::GraphicsObject()
    :
    mType(GT_NONE),
    mName("")
{
}

GraphicsObject::GraphicsObject(GraphicsObjectType type)
    :
    mType(type),
    mName("")
{
}

void GraphicsObject::SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.insert(listener);
    }
    msLFDMutex.unlock();
}

void GraphicsObject::UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.erase(listener);
    }
    msLFDMutex.unlock();
}

std::mutex GraphicsObject::msLFDMutex{};
std::set<std::shared_ptr<GraphicsObject::ListenerForDestruction>> GraphicsObject::msLFDSet{};
