// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46AtomicCounterBuffer.h>
using namespace gte;

GL46AtomicCounterBuffer::GL46AtomicCounterBuffer(RawBuffer const* cbuffer)
    :
    GL46Buffer(cbuffer, GL_ATOMIC_COUNTER_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL46AtomicCounterBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_RAW_BUFFER)
    {
        return std::make_shared<GL46AtomicCounterBuffer>(
            static_cast<RawBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL46AtomicCounterBuffer::AttachToUnit(GLint atomicCounterBufferUnit)
{
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBufferUnit, mGLHandle);
}


