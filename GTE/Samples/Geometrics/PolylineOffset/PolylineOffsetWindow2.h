// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.6.2022.12.24

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/PolylineOffset.h>
using namespace gte;

class PolylineOffsetWindow2 : public Window2
{
public:
    PolylineOffsetWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual void DrawScreenOverlay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    std::vector<Vector2<double>> mVertices;
    bool mIsOpen;
    std::shared_ptr<PolylineOffset<double>> mOffseter;
    std::vector<Vector2<double>> mRightPolyline, mLeftPolyline;
    double mOffsetDistance;
};
