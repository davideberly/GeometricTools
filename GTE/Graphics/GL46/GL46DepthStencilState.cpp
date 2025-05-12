// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46DepthStencilState.h>
using namespace gte;

GL46DepthStencilState::GL46DepthStencilState(DepthStencilState const* depthStencilState)
    :
    GL46DrawingState(depthStencilState)
{
    mDepthEnable = (depthStencilState->depthEnable ? GL_TRUE : GL_FALSE);
    mWriteMask = msWriteMask[depthStencilState->writeMask];
    mComparison = msComparison[depthStencilState->comparison];
    mStencilEnable = (depthStencilState->stencilEnable ? GL_TRUE : GL_FALSE);
    mStencilReadMask = depthStencilState->stencilReadMask;
    mStencilWriteMask = depthStencilState->stencilWriteMask;
    DepthStencilState::Face front = depthStencilState->frontFace;
    mFrontFace.onFail = msOperation[front.fail];
    mFrontFace.onZFail = msOperation[front.depthFail];
    mFrontFace.onZPass = msOperation[front.pass];
    mFrontFace.comparison = msComparison[front.comparison];
    DepthStencilState::Face back = depthStencilState->backFace;
    mBackFace.onFail = msOperation[back.fail];
    mBackFace.onZFail = msOperation[back.depthFail];
    mBackFace.onZPass = msOperation[back.pass];
    mBackFace.comparison = msComparison[back.comparison];
    mReference = depthStencilState->reference;
}

std::shared_ptr<GEObject> GL46DepthStencilState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_DEPTH_STENCIL_STATE)
    {
        return std::make_shared<GL46DepthStencilState>(
            static_cast<DepthStencilState const*>(object));
    }

    LogError("Invalid object type.");
}

void GL46DepthStencilState::Enable()
{
    if (mDepthEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(mComparison);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glDepthMask(mWriteMask);

    if (mStencilEnable)
    {
        glEnable(GL_STENCIL_TEST);

        glStencilFuncSeparate(GL_FRONT, mFrontFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_FRONT, mStencilWriteMask);
        glStencilOpSeparate(GL_FRONT, mFrontFace.onFail, mFrontFace.onZFail, mFrontFace.onZPass);
        glStencilFuncSeparate(GL_BACK, mBackFace.comparison, mReference, mStencilReadMask);
        glStencilMaskSeparate(GL_BACK, mStencilWriteMask);
        glStencilOpSeparate(GL_BACK, mBackFace.onFail, mBackFace.onZFail, mBackFace.onZPass);
    }
    else
    {
        glDisable(GL_STENCIL_TEST);
    }
}


GLboolean const GL46DepthStencilState::msWriteMask[] =
{
    GL_FALSE,
    GL_TRUE
};

GLenum const GL46DepthStencilState::msComparison[] =
{
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS
};

GLenum const GL46DepthStencilState::msOperation[] =
{
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_DECR,  // TODO: DX11 has INCR_SAT that clamps the result
    GL_INVERT,
    GL_INCR,
    GL_DECR
};

