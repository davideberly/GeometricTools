// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Graphics/VertexBuffer.h>
#include <Graphics/GL46/GL46Buffer.h>

namespace gte
{
    class GL46VertexBuffer : public GL46Buffer
    {
    public:
        // Construction.
        virtual ~GL46VertexBuffer() = default;
        GL46VertexBuffer(VertexBuffer const* vbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline VertexBuffer* GetVertexBuffer() const
        {
            return static_cast<VertexBuffer*>(mGTObject);
        }

        // TODO: Drawing support?  Currently, the enable/disable is in the
        // GL46InputLayout class, which assumes OpenGL 4.6 or later.  What if
        // the application machine does not have OpenGL 4.6?  Fall back to the
        // glBindBuffer paradigm?
    };
}

