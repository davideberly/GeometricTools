// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.12.26

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Camera.h>
#include <Mathematics/Logger.h>
using namespace gte;

Camera::Camera(bool isPerspective, bool isDepthRangeZeroOne)
    :
    ViewVolume(isPerspective, isDepthRangeZeroOne),
    mPreViewMatrix(Matrix4x4<float>::Identity()),
    mPostProjectionMatrix(Matrix4x4<float>::Identity()),
    mPreViewIsIdentity(true),
    mPostProjectionIsIdentity(true)
{
}

void Camera::SetPreViewMatrix(Matrix4x4<float> const& preViewMatrix)
{
    mPreViewMatrix = preViewMatrix;
    mPreViewIsIdentity = (mPreViewMatrix == Matrix4x4<float>::Identity());
    UpdatePVMatrix();
}

void Camera::SetPostProjectionMatrix(Matrix4x4<float> const& postProjMatrix)
{
    mPostProjectionMatrix = postProjMatrix;
    mPostProjectionIsIdentity = (mPostProjectionMatrix == Matrix4x4<float>::Identity());
    UpdatePVMatrix();
}

bool Camera::GetPickLine(int32_t viewX, int32_t viewY, int32_t viewW, int32_t viewH, int32_t x, int32_t y,
    Vector4<float>& origin, Vector4<float>& direction) const
{
    if (viewX <= x && x <= viewX + viewW && viewY <= y && y <= viewY + viewH)
    {
        // Get the [0,1]^2-normalized coordinates of (x,y).
        float r = (static_cast<float>(x - viewX)) / static_cast<float>(viewW);
        float u = (static_cast<float>(y - viewY)) / static_cast<float>(viewH);

        // Get the relative coordinates in [rmin,rmax]x[umin,umax].
        float rBlend = (1.0f - r) * GetRMin() + r * GetRMax();
        float uBlend = (1.0f - u) * GetUMin() + u * GetUMax();

        if (IsPerspective())
        {
            origin = GetPosition();
            direction = GetDMin() * GetDVector() + rBlend * GetRVector() + uBlend * GetUVector();
            Normalize(direction);
        }
        else
        {
            origin = GetPosition() + rBlend * GetRVector() + uBlend * GetUVector();
            direction = GetDVector();
        }
        return true;
    }
    else
    {
        // (x,y) is outside the viewport.
        return false;
    }
}

void Camera::UpdatePVMatrix()
{
    ViewVolume::UpdatePVMatrix();

    Matrix4x4<float>& pvMatrix = mProjectionViewMatrix;

#if defined(GTE_USE_VEC_MAT)
    if (!mPostProjectionIsIdentity)
    {
        pvMatrix = pvMatrix * mPostProjectionMatrix;
    }
    if (!mPreViewIsIdentity)
    {
        pvMatrix = mPreViewMatrix * pvMatrix;
    }
#else
    if (!mPostProjectionIsIdentity)
    {
        pvMatrix = mPostProjectionMatrix * pvMatrix;
    }
    if (!mPreViewIsIdentity)
    {
        pvMatrix = pvMatrix * mPreViewMatrix;
    }
#endif
}
