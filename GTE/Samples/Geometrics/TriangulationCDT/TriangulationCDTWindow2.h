// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.10.26

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/TriangulateCDT.h>
using namespace gte;

class TriangulationCDTWindow2 : public Window2
{
public:
    TriangulationCDTWindow2(Parameters& parameters);

    virtual bool OnCharPress(unsigned char key, int x, int y) override;

private:
    void DrawTriangulation();

    void UnindexedSimplePolygon();  // key = '0'
    void IndexedSimplePolygon();    // key = '1'
    void OneNestedPolygon();        // key = '2'
    void TwoNestedPolygons();       // key = '3'
    void TreeOfNestedPolygons();    // key = '4'

    static size_t constexpr smax = std::numeric_limits<size_t>::max();
    typedef BSNumber<UIntegerAP32> Rational;

    std::vector<Vector2<float>> mPoints;
    TriangulateCDT<float, Rational> mTriangulator;
    PolygonTreeEx mOutput;
};
