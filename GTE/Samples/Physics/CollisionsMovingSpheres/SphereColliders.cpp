// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.3.2023.08.08

#include <Mathematics/Logger.h>
#include "SphereColliders.h"
#include <limits>

SphereColliders::SphereColliders(Sphere3<float> const& sphere0,
    Sphere3<float> const& sphere1)
    :
    mSphere0(&sphere0),
    mSphere1(&sphere1),
    mContactPoint
    {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    }
{
}

float SphereColliders::Pseudodistance(float time,
    Vector3<float> const& velocity0, Vector3<float> const& velocity1) const
{
    Vector3<float> movedCenter0 = mSphere0->center + time * velocity0;
    Vector3<float> movedCenter1 = mSphere1->center + time * velocity1;
    Vector3<float> diff = movedCenter1 - movedCenter0;
    float sqrDistance = Dot(diff, diff);
    float rSum = mSphere0->radius + mSphere1->radius;
    float pseudodistance = sqrDistance / (rSum * rSum) - 1.0f;
    return pseudodistance;
}

void SphereColliders::ComputeContactInformation(CollisionType type,
    float time, Vector3<float> const&, Vector3<float> const&)
{
    if (type == CollisionType::SEPARATED)
    {
        float constexpr infinity = std::numeric_limits<float>::max();
        mContactTime = infinity;
        mContactPoint = { infinity, infinity, infinity };
    }
    else if (type == CollisionType::TOUCHING)
    {
        mContactTime = time;
        Vector3<float> diff = mSphere1->center - mSphere0->center;
        Normalize(diff);
        mContactPoint = mSphere0->center + mSphere0->radius * diff;
    }
    else if (type == CollisionType::OVERLAPPING)
    {
        // Just return the midpoint of the line segment connecting centers.
        // The actual contact set is either a circle, or one sphere is
        // contained in the other sphere.
        mContactTime = 0.0f;
        mContactPoint = 0.5f * (mSphere0->center + mSphere1->center);
    }
    else
    {
        LogError("The type cannot be CollisionType::UNKNOWN.");
    }
}
