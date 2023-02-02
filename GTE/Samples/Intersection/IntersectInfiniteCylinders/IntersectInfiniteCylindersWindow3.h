// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window3.h>
using namespace gte;

// Computing the curve of intersection of two (hollow) cylinders is described
// in the document
// https://www.geometrictools.com/Documentation/IntersectionInfiniteCylinders.pdf
// TODO: Factor out the intersection code into a FIQuery object (an object to
// manage the find-intersection query).

class IntersectInfiniteCylindersWindow3 : public Window3
{
public:
    IntersectInfiniteCylindersWindow3(Parameters& parameters);

    virtual void OnIdle() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void CreateScene();

    std::shared_ptr<RasterizerState> mWireState;
    std::shared_ptr<Visual> mCylinder0, mCylinder1;
    std::shared_ptr<Visual> mCurve0, mCurve1;
    float mC0, mW1, mW2;
    float mRadius0, mRadius1, mHeight, mAngle;
};
