// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/ConstrainedDelaunay2.h>
using namespace gte;

// After the program launches, press the key '0' to see a constrained edge
// inserted into the triangulation.  Then press key '1', and then press
// key '2'.

class ConstrainedDelaunay2DWindow2 : public Window2
{
public:
    ConstrainedDelaunay2DWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;
    virtual bool OnMouseClick(MouseButton button, MouseState state, int32_t x, int32_t y, uint32_t modifiers) override;

private:
    std::vector<Vector2<float>> mVertices;
    std::vector<int32_t> mHull;
    int32_t mCurrentTriX, mCurrentTriY;

    // The choice of N = 5 is sufficient for the data set generated in this
    // example.  Generally, it has to be larger.
    ConstrainedDelaunay2<float, BSNumber<UIntegerFP32<5>>> mDelaunay;
    ConstrainedDelaunay2<float, BSNumber<UIntegerFP32<5>>>::SearchInfo mInfo;
};
