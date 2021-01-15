// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Picker.h>
using namespace gte;

class PickingWindow3 : public Window3
{
public:
    PickingWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnMouseClick(MouseButton button, MouseState state, int x, int y, unsigned int modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void DoPick(int x, int y);

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mTorus;
    std::shared_ptr<Visual> mDodecahedron;
    std::shared_ptr<Visual> mPoints;
    std::shared_ptr<Visual> mSegments;
    enum { SPHERE_BUDGET = 16 };
    std::shared_ptr<Visual> mSphere[SPHERE_BUDGET];
    int mNumActiveSpheres;
    Picker mPicker;
};
