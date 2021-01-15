// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include "BipedManager.h"
using namespace gte;

class BlendedAnimationsWindow3 : public Window3
{
public:
    BlendedAnimationsWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnKeyDown(int key, int x, int y) override;
    virtual bool OnKeyUp(int key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void GetMeshes(std::shared_ptr<Spatial> const& object);
    void Update();

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mFloor;
    std::shared_ptr<RasterizerState> mWireState;
    std::vector<Visual*> mMeshes;
    std::unique_ptr<BipedManager> mManager;
    double mApplicationTime, mApplicationTimeDelta;
    bool mUpArrowPressed, mShiftPressed;
};
