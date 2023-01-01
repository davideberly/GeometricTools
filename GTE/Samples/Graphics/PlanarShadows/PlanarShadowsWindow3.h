// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.2.2022.03.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/PlanarShadowEffect.h>
using namespace gte;

class PlanarShadowsWindow3 : public Window3
{
public:
    PlanarShadowsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

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
    std::shared_ptr<Node> mShadowCaster;
    std::shared_ptr<PlanarShadowEffect::LightProjector> mLightProjector;
    std::shared_ptr<PlanarShadowEffect> mPlanarShadowEffect;

    // The coordinates of the light-projector relative to the trackball
    // affine basis.
    Vector4<float> mLPPosition, mLPDirection;
};
