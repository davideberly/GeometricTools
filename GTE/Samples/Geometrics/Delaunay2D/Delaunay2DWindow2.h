// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/Delaunay2.h>
using namespace gte;

class Delaunay2DWindow2 : public Window2
{
public:
    Delaunay2DWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnMouseClick(MouseButton button, MouseState state,
        int x, int y, unsigned int modifiers) override;

private:
    std::vector<Vector2<float>> mVertices;
    std::vector<int> mHull;
    Delaunay2<float, BSNumber<UIntegerAP32>> mDelaunay;
    Delaunay2<float, BSNumber<UIntegerAP32>>::SearchInfo mInfo;
    int mCurrentTriX, mCurrentTriY, mCurrentIndex;
};
