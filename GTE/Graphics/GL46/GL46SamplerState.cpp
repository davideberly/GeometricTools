// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46SamplerState.h>
using namespace gte;

GL46SamplerState::~GL46SamplerState()
{
    glDeleteSamplers(1, &mGLHandle);
}

GL46SamplerState::GL46SamplerState(SamplerState const* samplerState)
    :
    GL46DrawingState(samplerState)
{
    glGenSamplers(1, &mGLHandle);

    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_S, msMode[samplerState->mode[0]]);
    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_T, msMode[samplerState->mode[1]]);
    glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_R, msMode[samplerState->mode[2]]);

    // TODO: GL_TEXTURE_MAX_ANISOTROPY_EXT is not defined?
    // glSamplerParameterf(samplerState, GL_TEXTURE_MAX_ANISOTROPY_EXT,
    //   samplerState->maxAnisotropy);

    glSamplerParameterf(mGLHandle, GL_TEXTURE_MIN_LOD, samplerState->minLOD);
    glSamplerParameterf(mGLHandle, GL_TEXTURE_MAX_LOD, samplerState->maxLOD);
    glSamplerParameterf(mGLHandle, GL_TEXTURE_LOD_BIAS, samplerState->mipLODBias);

    glSamplerParameterfv(mGLHandle, GL_TEXTURE_BORDER_COLOR, &samplerState->borderColor[0]);

    switch(samplerState->filter)
    {
    case SamplerState::Filter::MIN_P_MAG_P_MIP_P:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case SamplerState::Filter::MIN_P_MAG_P_MIP_L:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case SamplerState::Filter::MIN_P_MAG_L_MIP_P:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case SamplerState::Filter::MIN_P_MAG_L_MIP_L:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case SamplerState::Filter::MIN_L_MAG_P_MIP_P:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case SamplerState::Filter::MIN_L_MAG_P_MIP_L:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case SamplerState::Filter::MIN_L_MAG_L_MIP_P:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case SamplerState::Filter::MIN_L_MAG_L_MIP_L:
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    default:
        LogError("Unknown sampler state filter.");
    }
}

std::shared_ptr<GEObject> GL46SamplerState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_SAMPLER_STATE)
    {
        return std::make_shared<GL46SamplerState>(
            static_cast<SamplerState const*>(object));
    }

    LogError("Invalid object type.");
}


GLint const GL46SamplerState::msMode[] =
{
    GL_REPEAT,          // WRAP
    GL_MIRRORED_REPEAT, // MIRROR
    GL_CLAMP_TO_EDGE,   // CLAMP
    GL_CLAMP_TO_BORDER, // BORDER
    GL_MIRRORED_REPEAT  // MIRROR_ONCE
};


