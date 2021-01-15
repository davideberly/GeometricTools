// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/NaturalSplineCurve.h>
using namespace gte;

class HelixTubeSurfaceWindow3 : public Window3
{
public:
    HelixTubeSurfaceWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;
    virtual bool OnKeyDown(int key, int x, int y) override;

private:
    bool SetEnvironment();
    void CreateScene();
    void CreateCurve();
    void MoveCamera(float time);

    struct Vertex
    {
        Vector3<float> position;
        Vector2<float> tcoord;
    };

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<NaturalSplineCurve<3, float>> mMedial;
    std::shared_ptr<Visual> mHelixTube;
    float mMinCurveTime, mMaxCurveTime, mCurvePeriod;
    float mCurveTime, mDeltaTime;
};
