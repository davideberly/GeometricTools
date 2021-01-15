// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include "PhysicsModule.h"
using namespace gte;

class KeplerPolarFormWindow2 : public Window2
{
public:
    KeplerPolarFormWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    PhysicsModule mModule;
    std::vector<Vector2<float>> mPositions;
    int mSize;
};
