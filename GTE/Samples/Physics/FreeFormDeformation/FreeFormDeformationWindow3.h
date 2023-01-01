// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
#include <Applications/Timer.h>
#include <Graphics/Picker.h>
#include <Mathematics/BSplineVolume.h>
using namespace gte;

class FreeFormDeformationWindow3 : public Window3
{
public:
    FreeFormDeformationWindow3(Parameters& parameters);

    virtual void OnIdle() override;

    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int32_t x, int32_t y, uint32_t modifiers) override;

    virtual bool OnMouseMotion(MouseButton button, int32_t x, int32_t y,
        uint32_t modifiers) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateBSplineVolume();
    void CreateSegments();
    void CreateBoxes();

    void UpdateMesh();
    void UpdateSegments();
    void UpdateBoxes();

    void DoRandomControlPoints();
    void OnMouseDown(int32_t x, int32_t y);
    void OnMouseMove(int32_t x, int32_t y);

    // The scene graph objects.
    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Node> mScene;
    std::shared_ptr<Visual> mMesh;
    Vector4<float> mRed, mGreen, mBlue, mGray;

    // The control volume for deformation.  The mParameters are the
    // (u,v,w) for the mesh vertices.
    int32_t mQuantity, mDegree;
    std::unique_ptr<BSplineVolume<3, float>> mVolume;
    Vector3<float> mMin, mMax, mDelta;
    std::vector<Vector3<float>> mParameters;

    // Q control points per dimension, 3*Q^2*(Q-1) polysegments to connect them.
    std::shared_ptr<Node> mPolysegmentRoot;
    std::vector<std::shared_ptr<Visual>> mSegments;

    // User-adjusted controls.
    std::shared_ptr<Node> mControlRoot;
    std::shared_ptr<Visual> mSelected;
    Vector4<float> mOldWorldPosition;
    std::vector<std::shared_ptr<Visual>> mBoxes;

    // Toggle between automated random motion and user-adjusted controls.
    Timer mMotionTimer;
    float mAmplitude, mRadius;
    double mLastUpdateTime;
    bool mDoRandom;

    // Toggle drawing of segments/boxes.
    bool mDrawSegmentsBoxes;

    // Picking support for selecting and moving the control points.
    Picker mPicker;
    bool mMouseDown;
};
