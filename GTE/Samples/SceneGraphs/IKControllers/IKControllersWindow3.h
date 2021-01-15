// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
using namespace gte;

class IKControllersWindow3 : public Window3
{
public:
    IKControllersWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnMouseMotion(MouseButton button, int x, int y,
        unsigned int modifiers) override;

private:
    void CreateScene();
    std::shared_ptr<Visual> CreateCube();
    std::shared_ptr<Visual> CreateRod();
    std::shared_ptr<Visual> CreateGround();
    void UpdateRod();
    bool Transform(unsigned char key);

    struct Vertex
    {
        Vector3<float> position;
        Vector4<float> color;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene, mIKSystem, mGoal, mJoint0, mJoint1;
    std::shared_ptr<Visual> mGround, mGoalCube, mOriginCube, mEndCube, mRod;
};
