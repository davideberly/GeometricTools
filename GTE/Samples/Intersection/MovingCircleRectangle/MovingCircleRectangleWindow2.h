// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/IntrOrientedBox2Circle2.h>
using namespace gte;

class MovingCircleRectangleWindow2 : public Window2
{
public:
    MovingCircleRectangleWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnMouseClick(int button, int state, int x, int y, unsigned int modifiers);
    virtual bool OnMouseMotion(int button, int x, int y, unsigned int modifiers);
    virtual bool OnCharPress(unsigned char key, int x, int y);

private:
    void DoQuery();
    void ModifyVelocity(int x, int y);
    void ModifyCircle(int x, int y);
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
