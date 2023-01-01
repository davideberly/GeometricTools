// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class MinimumVolumeBox3DWindow3 : public Window3
{
public:
    MinimumVolumeBox3DWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    void CreateScene();

    enum
    {
        NUM_POINTS = 2048
    };

    std::vector<Vector3<float>> mVertices;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mPolytope;
    std::shared_ptr<Visual> mBoxMesh;
    std::shared_ptr<RasterizerState> mWireState;
};
