// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Graphics/Controller.h>
#include <Mathematics/Transform.h>

namespace gte
{
    class TransformController : public Controller
    {
    public:
        TransformController(Transform<float> const& localTransform);

        // Member access.
        inline void SetTransform(Transform<float> const& localTransform)
        {
            mLocalTransform = localTransform;
        }

        inline Transform<float> const& GetTransform() const
        {
            return mLocalTransform;
        }

        // The animation update.  The application time is in milliseconds.
        // The update simply copies mLocalTransform to the Spatial mObject's
        // LocalTransform.  In this sense, TransformController represents a
        // transform that is constant for all time.
        virtual bool Update(double applicationTime) override;

    protected:
        Transform<float> mLocalTransform;
    };
}
