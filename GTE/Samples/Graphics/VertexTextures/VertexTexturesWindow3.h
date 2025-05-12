// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include "DisplacementEffect.h"
using namespace gte;

class VertexTexturesWindow3 : public Window3
{
public:
    VertexTexturesWindow3(Parameters& parameters);

    virtual void OnIdle();

private:
    bool SetEnvironment();
    void CreateMesh();

    std::shared_ptr<Visual> mHeightMesh;
    std::shared_ptr<DisplacementEffect> mEffect;
};

