// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2026
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class WireMeshWindow3 : public Window3
{
public:
    WireMeshWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    bool CreateScene();

    std::shared_ptr<Visual> mMesh;
};


