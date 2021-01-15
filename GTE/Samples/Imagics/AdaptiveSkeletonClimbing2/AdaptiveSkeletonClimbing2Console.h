// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Console.h>
using namespace gte;

class AdaptiveSkeletonClimbing2Console : public Console
{
public:
    AdaptiveSkeletonClimbing2Console(Parameters& parameters);

    virtual void Execute() override;

private:
    void Test0();
    void Test1();
};
