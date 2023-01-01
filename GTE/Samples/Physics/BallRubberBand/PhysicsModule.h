// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Mathematics/Vector2.h>
using namespace gte;

class PhysicsModule
{
public:
    // Construction.
    PhysicsModule();

    // Initialize the system.
    void Initialize(float time, float deltaTime,
        Vector2<float> const& initialPosition, Vector2<float> const& initialVelocity);

    // Apply a single step of the simulation.
    void Update ();

    // Access the current state.
    inline float GetTime() const
    {
        return mTime;
    }

    inline float GetDeltaTime() const
    {
        return mDeltaTime;
    }

    inline Vector2<float> const& GetPosition() const
    {
        return mPosition;
    }

    inline Vector2<float> const& GetVelocity() const
    {
        return mVelocity;
    }

    inline float GetFrequency() const
    {
        return mFrequency;
    }

    // Physical constants.
    float springConstant;  // c
    float mass;  // m

private:
    void Evaluate ();

    // State variables.
    float mTime, mDeltaTime;
    Vector2<float> mPosition, mVelocity;

    // Auxiliary variables.
    Vector2<float> mInitialPosition;
    float mFrequency;  // sqrt(c/m)
    Vector2<float> mVelDivFreq;  // initialVelocity/frequency
};
