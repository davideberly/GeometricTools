// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window3.h>
#include <Mathematics/NaturalSplineCurve.h>
using namespace gte;

class HelixTubeSurfaceWindow3 : public Window3
{
public:
    HelixTubeSurfaceWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnKeyDown(int32_t key, int32_t x, int32_t y) override;

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

