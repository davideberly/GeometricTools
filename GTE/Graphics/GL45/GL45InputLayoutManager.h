// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/GL45/GL45InputLayout.h>
#include <map>
#include <mutex>

namespace gte
{
    class GL45InputLayoutManager : public GEInputLayoutManager
    {
    public:
        // Construction and destruction.
        GL45InputLayoutManager()
            :
            mMap{},
            mMutex{}
        {
        }

        virtual ~GL45InputLayoutManager();

        // Management functions.  The Unbind(vbuffer) removes all layouts that
        // involve vbuffer.  The Unbind(vshader) is stubbed out because GL45
        // does not require it, but we wish to have
        // Unbind(GraphicsObject const*) as a base-class GraphicsEngine
        // function.
        GL45InputLayout* Bind(GLuint programHandle, GLuint vbufferHandle, VertexBuffer const* vbuffer);
        virtual bool Unbind(VertexBuffer const* vbuffer) override;
        virtual bool Unbind(Shader const* vshader) override;
        virtual void UnbindAll() override;
        virtual bool HasElements() const override;

    private:
        typedef std::pair<VertexBuffer const*, GLuint> VBPPair;
        std::map<VBPPair, std::shared_ptr<GL45InputLayout>> mMap;
        mutable std::mutex mMutex;
    };
}
