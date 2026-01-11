// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46Resource.h>
using namespace gte;

GL46Resource::GL46Resource(Resource const* gtResource)
    :
    GL46GraphicsObject(gtResource)
{
}

void* GL46Resource::MapForWrite(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    GLvoid* mapped = glMapBuffer(target, GL_WRITE_ONLY);
    glBindBuffer(target, 0);
    return mapped;
}

void GL46Resource::Unmap(GLenum target)
{
    glBindBuffer(target, mGLHandle);
    glUnmapBuffer(target);
    glBindBuffer(target, 0);
}

bool GL46Resource::PreparedForCopy(GLenum access) const
{
    // TODO: DX11 requires a staging resource when copying between CPU and
    // GPU.  Does OpenGL hide the double hop?

    // Verify existence of objects.
    LogAssert(mGLHandle != 0, "GL object does not exist.");

    // Verify the copy type.
    uint32_t copyType = GetResource()->GetCopy();
    if (copyType == Resource::Copy::CPU_TO_STAGING)  // CPU -> GPU
    {
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::Copy::STAGING_TO_CPU)  // GPU -> CPU
    {
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::Copy::BIDIRECTIONAL)
    {
        if (access == GL_READ_WRITE)
        {
            return true;
        }
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }

    LogError("Resource has incorrect copy type.");
}


