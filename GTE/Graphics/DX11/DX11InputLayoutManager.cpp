// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/DX11InputLayoutManager.h>
using namespace gte;

DX11InputLayoutManager::~DX11InputLayoutManager()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

DX11InputLayout* DX11InputLayoutManager::Bind(ID3D11Device* device,
    VertexBuffer const* vbuffer, Shader const* vshader)
{
    LogAssert(vshader != nullptr, "Invalid input.");

    if (vbuffer)
    {
        mMutex.lock();
        VBSPair vbs(vbuffer, vshader);
        auto iter = mMap.find(vbs);
        if (iter == mMap.end())
        {
            auto layout = std::make_shared<DX11InputLayout>(device, vbuffer, vshader);
            iter = mMap.insert(std::make_pair(vbs, layout)).first;

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
            std::string vbname = vbuffer->GetName();
            std::string vsname = vshader->GetName();
            if (vbname != "" || vsname != "")
            {
                layout->SetName(vbname + " | " + vsname);
            }
#endif
        }
        DX11InputLayout* inputLayout = iter->second.get();
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

bool DX11InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    LogAssert(vbuffer != nullptr, "Invalid input.");

    mMutex.lock();
    if (mMap.size() > 0)
    {
        std::vector<VBSPair> matches{};
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

bool DX11InputLayoutManager::Unbind(Shader const* vshader)
{
    LogAssert(vshader != nullptr, "Invalid input.");

    mMutex.lock();
    if (mMap.size() > 0)
    {
        std::vector<VBSPair> matches{};
        for (auto const& element : mMap)
        {
            if (vshader == element.first.second)
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

void DX11InputLayoutManager::UnbindAll()
{
    mMutex.lock();
    mMap.clear();
    mMutex.unlock();
}

bool DX11InputLayoutManager::HasElements() const
{
    mMutex.lock();
    bool hasElements = mMap.size() > 0;
    mMutex.unlock();
    return hasElements;
}
