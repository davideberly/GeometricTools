// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2024
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2024.06.24

#pragma once

#include <Graphics/ViewVolume.h>

namespace gte
{

    class Camera : public ViewVolume
    {
    public:
        // Construction.  The depth range for DirectX is [0,1] and for OpenGL
        // is [-1,1].  For DirectX, set isDepthRangeZeroToOne to true.  For
        // OpenGL, set isDepthRangeZeroOne to false.
        Camera(bool isPerspective, bool isDepthRangeZeroOne);

        // The preview matrix is applied after the model-to-world but before
        // the view matrix.  It is used for transformations such as
        // reflections of world objects.  The default value is the identity
        // matrix.
        void SetPreViewMatrix(Matrix4x4<float> const& preViewMatrix);

        inline Matrix4x4<float> const& GetPreViewMatrix() const
        {
            return mPreViewMatrix;
        }

        inline bool PreViewIsIdentity() const
        {
            return mPreViewIsIdentity;
        }

        // The postprojection matrix is used for screen-space transformations
        // such as reflection of the rendered image.  The default value is the
        // identity matrix.
        void SetPostProjectionMatrix(Matrix4x4<float> const& postProjMatrix);

        inline Matrix4x4<float> const& GetPostProjectionMatrix() const
        {
            return mPostProjectionMatrix;
        }

        inline bool PostProjectionIsIdentity() const
        {
            return mPostProjectionIsIdentity;
        }

        // Compute a picking line from the left-handed screen coordinates
        // (x,y), the viewport, and the camera.  The output 'origin' is the
        // camera position and the 'direction' is a unit-length vector, both
        // in world coordinates.  The return value is 'true' iff (x,y) is in
        // the viewport.
        bool GetPickLine(int32_t viewX, int32_t viewY, int32_t viewW, int32_t viewH, int32_t x, int32_t y,
            Vector4<float>& origin, Vector4<float>& direction) const;

    private:
        // After modifying the pre-view matrix or post-projection matrix,
        // update the projection-view matrix to include these.
        virtual void UpdatePVMatrix();

        // The preview matrix for the camera.
        Matrix4x4<float> mPreViewMatrix;

        // The postprojection matrix for the camera.
        Matrix4x4<float> mPostProjectionMatrix;

        // Indicates whether a user has specified a nonidentity pre-view
        // matrix.
        bool mPreViewIsIdentity;

        // Indicates whether a user has specified a nonidentity
        // post-projection matrix.
        bool mPostProjectionIsIdentity;
    };
}
