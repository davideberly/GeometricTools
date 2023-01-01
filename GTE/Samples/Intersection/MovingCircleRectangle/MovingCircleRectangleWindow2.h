// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/IntrOrientedBox2Circle2.h>
using namespace gte;

class MovingCircleRectangleWindow2 : public Window2
{
public:
    MovingCircleRectangleWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnMouseClick(int32_t button, int32_t state, int32_t x, int32_t y, uint32_t modifiers) override;
    virtual bool OnMouseMotion(int32_t button, int32_t x, int32_t y, uint32_t modifiers) override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void DoQuery();
    void ModifyVelocity(int32_t x, int32_t y);
    void ModifyCircle(int32_t x, int32_t y);
    void ModifyRectangle(double direction);

    OrientedBox2<double> mBox;
    Vector2<double> mBoxVelocity;
    Circle2<double> mCircle;
    Vector2<double> mCircleVelocity;
    double mContactTime;
    Vector2<double> mContactPoint;
    FIQuery<double, OrientedBox2<double>, Circle2<double>> mQuery;
    bool mLeftMouseDown, mRightMouseDown, mHasIntersection;
};
