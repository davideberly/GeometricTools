// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/GL46/GL46InputLayout.h>
#include <map>
#include <mutex>

namespace gte
{
    class GL46InputLayoutManager : public GEInputLayoutManager
    {
    public:
        // Construction and destruction.
        GL46InputLayoutManager()
            :
            mMap{},
            mMutex{}
        {
        }

        virtual ~GL46InputLayoutManager();

        // Management functions.  The Unbind(vbuffer) removes all layouts that
        // involve vbuffer.  The Unbind(vshader) is stubbed out because GL46
        // does not require it, but we wish to have
        // Unbind(GraphicsObject const*) as a base-class GraphicsEngine
        // function.
        GL46InputLayout* Bind(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);
        virtual bool Unbind(VertexBuffer const* vbuffer) override;
        virtual bool Unbind(Shader const* vshader) override;
        virtual void UnbindAll() override;
        virtual bool HasElements() const override;

    private:
        typedef std::pair<VertexBuffer const*, GLuint> VBPPair;
        std::map<VBPPair, std::shared_ptr<GL46InputLayout>> mMap;
        mutable std::mutex mMutex;
    };
}


