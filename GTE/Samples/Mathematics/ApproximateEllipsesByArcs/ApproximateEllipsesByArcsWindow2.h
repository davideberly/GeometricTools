// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
using namespace gte;

class ApproximateEllipsesByArcsWindow2 : public Window2
{
public:
    ApproximateEllipsesByArcsWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    // The ellipse extents 'a' and 'b' in (x/a)^2 + (y/b)^2 = 1.
    float mA, mB;

    // The arcs that approximate the ellipse.
    int32_t mNumArcs;
    std::vector<Vector2<float>> mPoints, mCenters;
    std::vector<float> mRadii;

    // For mapping ellipse points to screen coordinates.
    float mMultiplier;
    Vector2<float> mOffset;
};
