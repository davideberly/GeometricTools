// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/NURBSCurve.h>
using namespace gte;

class NURBSCurveExampleWindow2 : public Window2
{
public:
    NURBSCurveExampleWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void DoSimulation1();
    void DoSimulation2();
    void InitialConfiguration();
    void NextConfiguration();

    inline void Get(Vector2<float> const& position, int& x, int& y)
    {
        x = static_cast<int>(position[0] + 0.5f);
        y = mSize - 1 - static_cast<int>(position[1] + 0.5f);
    }

    std::shared_ptr<NURBSCurve<2, float>> mSpline;
    std::shared_ptr<NURBSCurve<2, float>> mCircle;
    std::vector<Vector2<float>> mControls;
    std::vector<Vector2<float>> mTargets;
    int mSize;
    float mH, mD;
    float mSimTime, mSimDelta;
    bool mDrawControlPoints;
};
