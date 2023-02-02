// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.03.20

#pragma once

#include <Mathematics/Vector3.h>
using namespace gte;

// An implementation of the abstract base class of Section 8.3.1 of "3D Game
// Engine Design, 2nd edition".

class Colliders
{
public:
    // Construciton and destruction.
    Colliders(
        float derivativeTimeStep = 1e-3f,
        float pseudodistanceThreshold = 1e-6f,
        size_t maxIterations = 8);

    virtual ~Colliders() = default;

    // Member access.
    void SetDerivativeTimeStep(float timeStep);
    void SetPseudodistanceThreshold(float threshold);
    void SetMaxIterations(size_t maxIterations);

    inline float GetDerivativeTimeStep() const
    {
        return mDerivativeTimeStep;
    }

    inline float GetPseudodistanceThreshold() const
    {
        return mPseudodistanceThreshold;
    }

    inline size_t GetMaxIterations() const
    {
        return mMaxIterations;
    }

    enum class CollisionType
    {
        UNKNOWN,
        SEPARATED,
        TOUCHING,
        OVERLAPPING
    };

    // The test-intersection query.
    virtual CollisionType Test(
        float maxTime,
        Vector3<float> const& velocity0,
        Vector3<float> const& velocity1,
        float& contactTime);

    // The find intersection query.
    virtual CollisionType Find(
        float maxTime,
        Vector3<float> const& velocity0,
        Vector3<float> const& velocity1,
        float& contactTime);

    // Return the contact time computed by a call to Test or Find. If there
    // is no contact, the returned time is std::numeric_limits<float>::max().
    // If the objects are overlapping, the returned time is 0.0f.
    inline float GetContactTime() const
    {
        return mContactTime;
    }

protected:
    virtual float Pseudodistance(float time, Vector3<float> const& velocity0,
        Vector3<float> const& velocity1) const = 0;

    virtual void ComputeContactInformation(CollisionType type, float time,
        Vector3<float> const& velocity0, Vector3<float> const& velocity1) = 0;

    virtual float PseudodistanceDerivative(float t0, float f0,
        Vector3<float> const& velocity0, Vector3<float> const& velocity1) const;

    virtual CollisionType FastNoIntersection(float maxTime,
        Vector3<float> const& velocity0, Vector3<float> const& velocity1,
        float& f0, float& fder0);

    float mDerivativeTimeStep;
    float mInvDerivativeTimeStep;
    float mPseudodistanceThreshold;
    size_t mMaxIterations;
    float mContactTime;
};
