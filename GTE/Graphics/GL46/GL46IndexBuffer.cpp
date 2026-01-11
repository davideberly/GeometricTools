// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46IndexBuffer.h>
using namespace gte;

GL46IndexBuffer::GL46IndexBuffer(IndexBuffer const* ibuffer)
    :
    GL46Buffer(ibuffer, GL_ELEMENT_ARRAY_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL46IndexBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_INDEX_BUFFER)
    {
        return std::make_shared<GL46IndexBuffer>(
            static_cast<IndexBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL46IndexBuffer::Enable()
{
    glBindBuffer(mType, mGLHandle);
}

void GL46IndexBuffer::Disable()
{
    glBindBuffer(mType, 0);
}


