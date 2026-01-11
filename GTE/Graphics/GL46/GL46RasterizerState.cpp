// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#include <Graphics/GL46/GTGraphicsGL46PCH.h>
#include <Graphics/GL46/GL46RasterizerState.h>
using namespace gte;

GL46RasterizerState::GL46RasterizerState(RasterizerState const* rasterizerState)
    :
    GL46DrawingState(rasterizerState)
{
    mFillMode = msFillMode[rasterizerState->fill];
    mCullFace = msCullFace[rasterizerState->cull];
    mFrontFace = (rasterizerState->frontCCW ? GL_CCW : GL_CW);
    mDepthScale = rasterizerState->slopeScaledDepthBias;
    mDepthBias = static_cast<float>(rasterizerState->depthBias);
    mEnableScissor = (rasterizerState->enableScissor ? GL_TRUE : GL_FALSE);
}

std::shared_ptr<GEObject> GL46RasterizerState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_RASTERIZER_STATE)
    {
        return std::make_shared<GL46RasterizerState>(
            static_cast<RasterizerState const*>(object));
    }

    LogError("Invalid object type.");
}

void GL46RasterizerState::Enable()
{
    glPolygonMode(GL_FRONT_AND_BACK, mFillMode);

    if (mCullFace != 0)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(mFrontFace);
        glCullFace(mCullFace);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    if (mDepthScale != 0.0f && mDepthBias != 0.0f)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_POLYGON_OFFSET_POINT);
        glPolygonOffset(mDepthScale, mDepthBias);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_POLYGON_OFFSET_POINT);
    }
}


GLenum const GL46RasterizerState::msFillMode[] =
{
    GL_FILL,
    GL_LINE
};

GLenum const GL46RasterizerState::msCullFace[] =
{
    0,
    GL_FRONT,
    GL_BACK
};


