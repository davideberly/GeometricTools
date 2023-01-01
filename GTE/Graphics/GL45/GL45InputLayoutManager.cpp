// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/GL45/GTGraphicsGL45PCH.h>
#include <Mathematics/Logger.h>
#include <Graphics/GL45/GL45InputLayoutManager.h>
using namespace gte;

GL45InputLayoutManager::~GL45InputLayoutManager()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

GL45InputLayout* GL45InputLayoutManager::Bind(GLuint programHandle,
    GLuint vbufferHandle, VertexBuffer const* vbuffer)
{
    LogAssert(programHandle != 0, "Invalid input.");

    if (vbuffer)
    {
        mMutex.lock();
        VBPPair vbp(vbuffer, programHandle);
        auto iter = mMap.find(vbp);
        if (iter == mMap.end())
        {
            auto layout = std::make_shared<GL45InputLayout>(programHandle, vbufferHandle, vbuffer);
            iter = mMap.insert(std::make_pair(vbp, layout)).first;
        }
        GL45InputLayout* inputLayout = iter->second.get();
        mMutex.unlock();
        return inputLayout;
    }
    else
    {
        // A null vertex buffer is passed when an effect wants to bypass the
        // input assembler.
        return nullptr;
    }
}

bool GL45InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    LogAssert(vbuffer != nullptr, "Invalid input.");

    mMutex.lock();
    if (mMap.size() > 0)
    {
        std::vector<VBPPair> matches{};
        for (auto const& element : mMap)
        {
            if (vbuffer == element.first.first)
            {
                matches.push_back(element.first);
            }
        }

        for (auto const& match : matches)
        {
            mMap.erase(match);
        }
    }
    mMutex.unlock();
    return true;
}

bool GL45InputLayoutManager::Unbind(Shader const*)
{
    return true;
}

void GL45InputLayoutManager::UnbindAll()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

bool GL45InputLayoutManager::HasElements() const
{
    mMutex.lock();
    bool hasElements = mMap.size() > 0;
    mMutex.unlock();
    return hasElements;
}
