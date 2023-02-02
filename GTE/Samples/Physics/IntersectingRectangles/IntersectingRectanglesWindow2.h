// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Applications/Timer.h>
#include <Mathematics/RectangleManager.h>
#include <random>
using namespace gte;

class IntersectingRectanglesWindow2 : public Window2
{
public:
    IntersectingRectanglesWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void OnIdle() override;

private:
    void ModifyRectangles();
    void DrawRectangles();

    std::vector<AlignedBox2<float>> mRectangles;
    std::unique_ptr<RectangleManager<float>> mManager;
    float mSize;
    Timer mTimer;
    double mLastIdle;
    std::mt19937 mMTE;
    std::uniform_real_distribution<float> mPerturb;
};
