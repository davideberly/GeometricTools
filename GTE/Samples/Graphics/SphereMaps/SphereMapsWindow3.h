// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/SphereMapEffect.h>
using namespace gte;

class SphereMapsWindow3 : public Window3
{
public:
    SphereMapsWindow3(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateScene();
    void UpdateConstants();

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mTorus;
    std::shared_ptr<SphereMapEffect> mSMEffect;
};
