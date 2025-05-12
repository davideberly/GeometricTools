// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46ConstantBuffer.h>
using namespace gte;

GL46ConstantBuffer::GL46ConstantBuffer(ConstantBuffer const* cbuffer)
    :
    GL46Buffer(cbuffer, GL_UNIFORM_BUFFER)
{
    Initialize();
}

std::shared_ptr<GEObject> GL46ConstantBuffer::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_CONSTANT_BUFFER)
    {
        return std::make_shared<GL46ConstantBuffer>(
            static_cast<ConstantBuffer const*>(object));
    }

    LogError("Invalid object type.");
}

void GL46ConstantBuffer::AttachToUnit(GLint uniformBufferUnit)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, uniformBufferUnit, mGLHandle);
}

