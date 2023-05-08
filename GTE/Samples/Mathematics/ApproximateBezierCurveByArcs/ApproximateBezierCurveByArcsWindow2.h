// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2023.05.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/BezierCurve.h>
#include <Mathematics/Arc2.h>
#include <functional>
using namespace gte;

class ApproximateBezierCurveByArcsWindow2 : public Window2
{
public:
    ApproximateBezierCurveByArcsWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay();
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y);

private:
    // The client window is [0,512]x[0,512], mXSize = mYSize = 512.
    // Transform [0,4]x[0,5] to client subwindow [32,432]x[32,532] by
    // multiplying the sample coordinates by 100 and adding 32.
    inline void Transform(Vector2<double> const& point, int32_t& x, int32_t& y)
    {
        x = static_cast<int32_t>(100.0 * point[0] + 32.0);
        y = static_cast<int32_t>(100.0 * point[1] + 32.0);
    }

    std::shared_ptr<BezierCurve<2, double>> mCurve;
    size_t mNumArcs;
    std::vector<double> mTimes;
    std::vector<Vector2<double>> mEndpoints;
    std::vector<Arc2<double>> mArcs;
    bool mDrawCurve, mDrawArcs, mDrawEndpoints, mDrawMidpoints, mDrawText;
};
