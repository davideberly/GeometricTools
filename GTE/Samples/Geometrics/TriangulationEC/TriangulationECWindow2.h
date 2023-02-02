// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2023
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 6.0.2022.01.06

#pragma once

#include <Applications/Window2.h>
#include <Mathematics/ArbitraryPrecision.h>
#include <Mathematics/TriangulateEC.h>
using namespace gte;

class TriangulationECWindow2 : public Window2
{
public:
    TriangulationECWindow2(Parameters& parameters);

    virtual void OnDisplay() override;
    virtual bool OnCharPress(uint8_t key, int32_t x, int32_t y) override;

private:
    void ClearAll();
    void UnindexedSimplePolygon();  // mExample = 0
    void IndexedSimplePolygon();    // mExample = 1
    void OneNestedPolygon();        // mExample = 2
    void TwoNestedPolygons();       // mExample = 3
    void TreeOfNestedPolygons();    // mExample = 4

    // The inputs (i0,i1,i2) are a permutation of (0,1,2).  The goal is to
    // trap algorithm errors due to order of inner polygons.
    // mExample = 5, 6, 7, 8, 9, 10
    void FourBoxesThreeNested(int32_t i0, int32_t i1, int32_t i2);

    typedef BSRational<UIntegerAP32> Rational;
    typedef TriangulateEC<float, Rational> Triangulator;

    std::vector<Vector2<float>> mPositions;
    std::vector<int32_t> mOuter, mInner0, mInner1, mInner2;
    std::shared_ptr<PolygonTree> mTree;
    std::vector<Vector2<float>> mFillSeeds;
    std::vector<std::array<int32_t, 3>> mTriangles;
    int32_t mExample;
};
