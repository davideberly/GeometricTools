// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/PlanarReflectionEffect.h>
using namespace gte;

class PlanarReflectionsWindow3 : public Window3
{
public:
    PlanarReflectionsWindow3(Parameters& parameters);

    virtual void OnIdle() override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateFloor();
    void CreateWall();
    void CreateDodecahedron();
    void CreateTorus();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    // The scene graph.
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mFloor, mWall, mDodecahedron, mTorus;
    std::shared_ptr<Node> mReflectionCaster;
    std::shared_ptr<PlanarReflectionEffect> mPlanarReflectionEffect;
};
