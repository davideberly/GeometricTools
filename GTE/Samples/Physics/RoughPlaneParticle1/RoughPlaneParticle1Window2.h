// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.12.14

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/Vector2.h>
#include "PhysicsModule.h"
using namespace gte;

// This is an implementation of an algorithm in Section 1 of
// https://www.geometrictools.com/Documentation/RoughPlaneAnalysis.pdf

class RoughPlaneParticle1Window2 : public Window2
{
public:
    RoughPlaneParticle1Window2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    PhysicsModule mModule;
    std::vector<Vector2<double>> mSFPositions;  // path with static friction
    bool mContinueSolving;

    // viscous solution:
    //   x(t) = a0*exp(-r*t)+a1
    //   w(t) = b0*exp(-r*t)+b1*t+b2
    //   r = c/m
    //   a0 = -xdot(0)/r
    //   a1 = x(0)-a0
    //   b1 = -g*sin(phi)/r
    //   b2 = (wdot(0)+r*w(0)-b1)/r
    //   b0 = w(0)-b2
    Vector2<double> GetVFPosition(double dTime);
    double mR, mA0, mA1, mB0, mB1, mB2;
    std::vector<Vector2<double>> mVFPositions;  // path with viscous friction
};
