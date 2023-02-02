// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.1.2022.01.10

#pragma once

#include <Applications/Window2.h>
#include "PhysicsModule.h"
using namespace gte;

// This is an implementation of an algorithm in Section 2 of
// https://www.geometrictools.com/Documentation/RoughPlaneAnalysis.pdf

class RoughPlaneParticle2Window2 : public Window2
{
public:
    RoughPlaneParticle2Window2(Parameters& parameters);

    virtual void OnIdle() override;
    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    PhysicsModule mModule;
    int32_t mIteration;
    int32_t mMaxIteration;
    int32_t mSize;
};
