// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Graphics/Picker.h>
using namespace gte;

class PickingWindow3 : public Window3
{
public:
    PickingWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnMouseClick(MouseButton button, MouseState state, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void DoPick(int32_t x, int32_t y);

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
    static int32_t constexpr SPHERE_BUDGET = 16;
    std::shared_ptr<Visual> mSphere[SPHERE_BUDGET];
    int32_t mNumActiveSpheres;
    Picker mPicker;
};
