// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window2.h>
#include "PhysicsModule.h"
using namespace gte;

class DoublePendulumWindow2 : public Window2
{
public:
    DoublePendulumWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void OnIdle() override;

private:
    PhysicsModule mModule;
    int32_t mSize;
};

