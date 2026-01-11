// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46Buffer.h>
using namespace gte;

GL46Buffer::~GL46Buffer()
{
    glDeleteBuffers(1, &mGLHandle);
}

GL46Buffer::GL46Buffer(Buffer const* buffer, GLenum type)
    :
    GL46Resource(buffer),
    mType(type)
{
    glGenBuffers(1, &mGLHandle);

    uint32_t usage = buffer->GetUsage();
    if (usage == Resource::Usage::IMMUTABLE)
    {
        mUsage = GL_STATIC_DRAW;
    }
    else if (usage == Resource::Usage::DYNAMIC_UPDATE)
    {
        mUsage = GL_DYNAMIC_DRAW;
    }
    else  // usage == Resource::Usage::SHADER_OUTPUT
    {
        // TODO: In GLSL, is it possible to write to a buffer other than a
        // vertex buffer?
        if (mType == GL_ARRAY_BUFFER)
        {
            mUsage = GL_STREAM_DRAW;
        }
        else if (mType == GL_SHADER_STORAGE_BUFFER)
        {
            mUsage = GL_DYNAMIC_DRAW;
        }
        else
        {
            // TODO: Can this buffer type be a shader output?
            mUsage = GL_STATIC_DRAW;
        }
    }
}

void GL46Buffer::Initialize()
{
    glBindBuffer(mType, mGLHandle);

    // Access the buffer.
    auto buffer = GetBuffer();

    // Create and initialize a buffer.
    glBufferData(mType, buffer->GetNumBytes(), buffer->GetData(), mUsage);

    glBindBuffer(mType, 0);
}

bool GL46Buffer::Update()
{
    Buffer* buffer = GetBuffer();
    LogAssert(buffer->GetUsage() == Resource::Usage::DYNAMIC_UPDATE,
        "Buffer usage is not DYNAMIC_UPDATE.");

    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        GLintptr offsetInBytes =
            static_cast<GLintptr>(buffer->GetOffset()) *
            static_cast<GLintptr>(buffer->GetElementSize());
        char const* source = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, offsetInBytes, numActiveBytes, source);
        glBindBuffer(mType, 0);
    }
    return true;
}

bool GL46Buffer::CopyCpuToGpu()
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
        GLintptr offsetInBytes =
            static_cast<GLintptr>(buffer->GetOffset()) *
            static_cast<GLintptr>(buffer->GetElementSize());
        char const* source = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glBufferSubData(mType, offsetInBytes, numActiveBytes, source);
        glBindBuffer(mType, 0);
    }
    return true;
}

bool GL46Buffer::CopyGpuToCpu()
{
    if (!PreparedForCopy(GL_READ_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    GLuint numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from GPU memory to CPU memory.
        GLintptr offsetInBytes =
            static_cast<GLintptr>(buffer->GetOffset()) *
            static_cast<GLintptr>(buffer->GetElementSize());
        char* target = buffer->GetData() + offsetInBytes;
        glBindBuffer(mType, mGLHandle);
        glGetBufferSubData(mType, offsetInBytes, numActiveBytes, target);
        glBindBuffer(mType, 0);
    }
    return true;
}


