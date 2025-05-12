// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Mathematics/Logger.h>
#include <Graphics/GL46/GL46InputLayoutManager.h>
using namespace gte;

GL46InputLayoutManager::~GL46InputLayoutManager()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

GL46InputLayout* GL46InputLayoutManager::Bind(GLuint programHandle,
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
            auto layout = std::make_shared<GL46InputLayout>(programHandle, vbufferHandle, vbuffer);
            iter = mMap.insert(std::make_pair(vbp, layout)).first;
        }
        GL46InputLayout* inputLayout = iter->second.get();
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

bool GL46InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
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

bool GL46InputLayoutManager::Unbind(Shader const*)
{
    return true;
}

void GL46InputLayoutManager::UnbindAll()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

bool GL46InputLayoutManager::HasElements() const
{
    mMutex.lock();
    bool hasElements = mMap.size() > 0;
    mMutex.unlock();
    return hasElements;
}

