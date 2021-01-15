// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/NURBSCircle.h>
using namespace gte;

class NURBSCircleWindow2 : public Window2
{
public:
    NURBSCircleWindow2(Parameters& parameters);

    virtual void OnDisplay() override;

private:
    void DrawCurve(NURBSCurve<2, float> const* curve, float maxAngle,
        int iXCenter, int iYCenter, int iRadius);

    NURBSQuarterCircleDegree2<float> mQuarterCircleDegree2;
    NURBSQuarterCircleDegree4<float> mQuarterCircleDegree4;
    NURBSHalfCircleDegree3<float> mHalfCircleDegree3;
    NURBSFullCircleDegree3<float> mFullCircleDegree3;
};
