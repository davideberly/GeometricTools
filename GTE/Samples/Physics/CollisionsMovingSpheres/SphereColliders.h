// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2022.03.20

#pragma once

#include <Mathematics/Hypersphere.h>
#include "Colliders.h"

// An implementation of a class derived from Colliders to illustrate
// intersection queries for spheres moving with constant linear velocity.
// The member functions are based on the discussion in Section 8.3.2 of
// "3D Game Engine Design, 2nd edition".

class SphereColliders : public Colliders
{
public:
    // Construction and destruction.
    SphereColliders(Sphere3<float> const& sphere0, Sphere3<float> const& sphere1);
    virtual ~SphereColliders() = default;

    // Call this function after a Test or Find call *and* when
    // GetContactTime() returns a value T such that 0 <= T <= maxTime,
    // where fMaxTime > 0 is the value supplied to the Test or Find call.
    inline Vector3<float> const& GetContactPoint() const
    {
        return mContactPoint;
    }

protected:
    virtual float Pseudodistance(float time,
        Vector3<float> const& velocity0,
        Vector3<float> const& velocity1) const override;

    virtual void ComputeContactInformation(CollisionType type, float time,
        Vector3<float> const& velocity0,
        Vector3<float> const& velocity1) override;

    Sphere3<float> const* mSphere0;
    Sphere3<float> const* mSphere1;
    Vector3<float> mContactPoint;
};
