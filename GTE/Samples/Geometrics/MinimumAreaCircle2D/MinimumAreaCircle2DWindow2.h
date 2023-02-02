// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/MinimumAreaCircle2.h>
using namespace gte;

class MinimumAreaCircle2DWindow2 : public Window2
{
public:
    MinimumAreaCircle2DWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    enum { NUM_POINTS = 256 };
    int32_t mNumActive;
    std::vector<Vector2<float>> mVertices;
    Circle2<float> mMinimalCircle;
    MinimumAreaCircle2<float, BSRational<UIntegerAP32>> mMAC2;
};
