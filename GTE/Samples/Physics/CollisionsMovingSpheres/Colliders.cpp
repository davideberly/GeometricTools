// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2023.08.08

#include "Colliders.h"
#include <limits>

Colliders::Colliders(float derivativeTimeStep,
    float pseudodistanceThreshold, size_t maxIterations)
    :
    mDerivativeTimeStep(0.0f),
    mInvDerivativeTimeStep(0.0f),
    mPseudodistanceThreshold(0.0f),
    mMaxIterations(0),
    mContactTime(std::numeric_limits<float>::max())
{
    SetDerivativeTimeStep(derivativeTimeStep);
    SetPseudodistanceThreshold(pseudodistanceThreshold);
    SetMaxIterations(maxIterations);
}

void Colliders::SetDerivativeTimeStep(float timeStep)
{
    if (timeStep > 0.0f)
    {
        mDerivativeTimeStep = timeStep;
    }
    else
    {
        mDerivativeTimeStep = 1e-03f;
    }

    mInvDerivativeTimeStep = 0.5f / mDerivativeTimeStep;
}

void Colliders::SetPseudodistanceThreshold(float threshold)
{
    if (threshold >= 0.0f)
    {
        mPseudodistanceThreshold = threshold;
    }
}

void Colliders::SetMaxIterations(size_t maxIterations)
{
    if (maxIterations > 0)
    {
        mMaxIterations = maxIterations;
    }
}

Colliders::CollisionType Colliders::Test(float maxTime,
    Vector3<float> const& velocity0, Vector3<float> const& velocity1,
    float& contactTime)
{
    return Find(maxTime, velocity0, velocity1, contactTime);
}

Colliders::CollisionType Colliders::Find(float maxTime,
    Vector3<float> const& velocity0, Vector3<float> const& velocity1,
    float& contactTime)
{
    float f0{}, fder0{};
    CollisionType type = FastNoIntersection(maxTime, velocity0, velocity1,
        f0, fder0);

    if (type == CollisionType::SEPARATED)
    {
        contactTime = std::numeric_limits<float>::max();
        return CollisionType::SEPARATED;
    }
    if (type == CollisionType::TOUCHING)
    {
        contactTime = 0.0f;
        return CollisionType::TOUCHING;
    }

    // Use Newton’s method for root finding when the derivative is calculated
    // but use the Secant method when the derivative is estimated.
    float t0 = 0.0f;
    for (size_t i = 1; i <= mMaxIterations; ++i)
    {
        t0 -= f0 / fder0;
        if (t0 > maxTime)
        {
            // The objects do not intersect during the specified time
            // interval.
            contactTime = std::numeric_limits<float>::max();
            return CollisionType::TOUCHING;
        }
        f0 = Pseudodistance(t0, velocity0, velocity1);
        if (f0 <= mPseudodistanceThreshold)
        {
            contactTime = t0;
            ComputeContactInformation(CollisionType::TOUCHING, contactTime,
                velocity0, velocity1);
            return CollisionType::TOUCHING;
        }
        fder0 = PseudodistanceDerivative(t0, f0, velocity0, velocity1);
        if (fder0 >= 0.0f)
        {
            // The objects are moving apart.
            contactTime = std::numeric_limits<float>::max();
            return CollisionType::SEPARATED;
        }
    }

    // Newton’s method failed to converge, but we already tested earlier
    // whether the objects were moving apart or not intersecting during the
    // specified time interval. To reach here, the number of iterations was
    // not large enough for the desired pseudodistance threshold. Most
    // likely this occurs when the relative speed is very large and the time
    // step for the derivative estimation needs to be smaller.
    contactTime = t0;
    ComputeContactInformation(CollisionType::TOUCHING, contactTime,
        velocity0, velocity1);
    return CollisionType::TOUCHING;
}

float Colliders::PseudodistanceDerivative(float t0, float f0,
    Vector3<float> const& velocity0, Vector3<float> const& velocity1) const
{
    float t1 = t0 - mDerivativeTimeStep;
    float f1 = Pseudodistance(t1, velocity0, velocity1);
    float fder0 = (f0 - f1) * mInvDerivativeTimeStep;
    return fder0;
}

Colliders::CollisionType Colliders::FastNoIntersection(float maxTime,
    Vector3<float> const& velocity0, Vector3<float> const& velocity1,
    float& f0, float& fder0)
{
    // Analyze the initial configuration of the objects.
    f0 = Pseudodistance(0.0f, velocity0, velocity1);
    fder0 = PseudodistanceDerivative(0.0f, f0, velocity0, velocity1);
    if (f0 <= -mPseudodistanceThreshold)
    {
        // Objects are (significantly) overlapping.
        ComputeContactInformation(CollisionType::OVERLAPPING, 0.0f,
            velocity0, velocity1);
        return (fder0 >= 0.0f ?
            CollisionType::SEPARATED : CollisionType::OVERLAPPING);
    }
    if (f0 <= mPseudodistanceThreshold)
    {
        // Objects are (nearly) in tangential contact.
        ComputeContactInformation(CollisionType::TOUCHING, 0.0f,
            velocity0, velocity1);
        return (fder0 >= 0.0f ?
            CollisionType::SEPARATED : CollisionType::TOUCHING);
    }

    // The objects are not currently in contact or overlapping. If the
    // objects are moving apart or the relative velocity between them is
    // zero, they cannot intersect at a later time.
    if (fder0 >= 0.0f || velocity0 == velocity1)
    {
        return CollisionType::SEPARATED;
    }

    // Check if the objects are not intersecting, yet still moving toward
    // each other at maximum time. If this is the case, the objects do not
    // intersect on the specified time interval.
    float f1 = Pseudodistance(maxTime, velocity0, velocity1);
    if (f1 > 0.0f)
    {
        // Compute or estimate the derivative F'(tmax).
        float fder1 = PseudodistanceDerivative(maxTime, f1,
            velocity0, velocity1);
        if (fder1 < 0.0f)
        {
            // The objects are moving toward each other and do not intersect
            // during the specified time interval.
            return CollisionType::SEPARATED;
        }
    }
    return CollisionType::UNKNOWN;
}
