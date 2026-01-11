// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46VertexBuffer.h>
using namespace gte;

GL46VertexBuffer::GL46VertexBuffer(VertexBuffer const* vbuffer)
    :
    GL46Buffer(vbuffer, GL_ARRAY_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL46VertexBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_VERTEX_BUFFER)
    {
        return std::make_shared<GL46VertexBuffer>(
            static_cast<VertexBuffer const*>(object));
    }

    LogError("Invalid object type.");
}


