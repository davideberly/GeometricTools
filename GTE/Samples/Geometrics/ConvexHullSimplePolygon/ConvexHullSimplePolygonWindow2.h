// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2025
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 8.0.2025.05.10

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ConvexHullSimplePolygon.h>
#include <Mathematics/PolygonWindingOrder.h>
#include <cstddef>
#include <cstdint>
#include <vector>
using namespace gte;

class ConvexHullSimplePolygonWindow2 : public Window2
{
public:
    ConvexHullSimplePolygonWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(std::uint8_t key, std::int32_t x, std::int32_t y) override;

private:
    void Polygon0StartConvex();
    void Polygon0StartReflex();
    void Polygon1StartConvex();
    void Polygon1StartReflex();

    std::vector<Vector2<float>> mPolygon;
    PolygonWindingOrder<float> mOrder;
    std::vector<std::size_t> mHull;
    ConvexHullSimplePolygon<float> mCHSP;
};

